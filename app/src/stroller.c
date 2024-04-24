#include "stroller.h"
#include "log.h"
#include "stdlib.h"
#include "tools.h"
#include <string.h>
#include <unistd.h>

#define TCP_SERVER_PORT 8888
#define TCP_SERVER_IP "0.0.0.0"
#define USE_IIC_BUS 3

static int is_stop = 0;

static bool stlr_is_automatable(stroller_t *stlr)
{
    stlr_sensor_t *sensor = &stlr->sensor;
    if (sensor_is_enabled(sensor->aht10) && sensor_is_enabled(sensor->mq135) && sensor_is_enabled(sensor->sw18015)) {
        return true;
    }
    return false;
}

static int stlr_comm_send(stroller_t *stlr, char *buf, int len)
{
    stlr_comm_t *comm = &stlr->comm;
    tcp_server_t *tcps = &comm->tcps;

    if (len > sizeof(comm->sbuf)) {
        log_error("comm send buffer overflow");
        return -1;
    }

    if (!tcp_is_connected(tcps)) {
        return -1;
    }

    pthread_mutex_lock(&comm->send_lock);
    /* Use queue to save buf in future*/
    if (strlen(comm->sbuf) != 0) {
        log_warn("comm send not ready");
        pthread_mutex_unlock(&comm->send_lock);
        return -1;
    }

    memcpy(comm->sbuf, buf, len);
    pthread_mutex_unlock(&comm->send_lock);
    return 0;
}

static void *sensor_loop(void *ptr)
{
    int ret = 0;
    stroller_t *stlr = ptr;
    stlr_sensor_t *sensor = &stlr->sensor;
    float temp, humi, co2;
    char buf[100];
    int shake;
    while (!is_stop) {
        ret = sensor_config(sensor->aht10, SENSOR_START_MEASURE, 0);
        ret |= sensor_read(sensor->aht10, &temp, SENSOR_CHANNEL0);
        ret |= sensor_read(sensor->aht10, &humi, SENSOR_CHANNEL1);
        if (ret < 0) {
            log_warn("Failed to read aht10");
        }

        ret = sensor_config(sensor->mq135, SENSOR_START_MEASURE, 0);
        ret |= sensor_read(sensor->mq135, &co2, SENSOR_CHANNEL0);
        if (ret < 0) {
            log_warn("Failed to read mq135");
        }

        ret = sensor_config(sensor->sw18015, SENSOR_START_MEASURE, 0);
        ret |= sensor_read(sensor->sw18015, &shake, SENSOR_CHANNEL0);
        if (ret < 0) {
            log_warn("Failed to read sw18015");
        }

        // log_debug("sensor read temp:%.1f, humi:%.1f, co2:%.1f, shake:%d", temp, humi, co2, shake);

        pthread_mutex_lock(&sensor->mutex);
        sensor->available = true;
        sensor->temp = temp;
        sensor->humi = humi;
        sensor->co2 = co2;
        sensor->shake = shake;
        snprintf(buf, sizeof(buf), "temp:%.1f, humi:%.1f, co2:%.1f, shake:%d\n", sensor->temp, sensor->humi, sensor->co2, sensor->shake);
        stlr_comm_send(stlr, buf, strlen(buf));
        pthread_mutex_unlock(&sensor->mutex);

        sleep(5);
    }
    return NULL;
}

static int move_of_remote(stroller_t *stlr, COMM_COMMAND cmd, int data1, int data2)
{
    int ret = 0;
    log_info("move_of_remote");
    chassis_t *chassis = &stlr->chassis;
    switch (cmd) {
        case COMM_CMD_STOP:
            set_chassis_speed(chassis, 0);
            log_debug("remote stop");
            break;
        case COMM_CMD_SPEED:
            set_chassis_speed(chassis, (float)data1);
            log_debug("remote forward");
            break;
        case COMM_CMD_DIRECTION:
            set_chassis_turn(chassis, (float)data1);
            log_debug("remote left");
            break;
        case COMM_CMD_NONE:
            /* Not something to do */
            break;
        default:
            log_error("not find remote command %d", cmd);
            ret = -1;
            break;
    }
    return ret;
}

static void *stlr_follow_loop(void *ptr)
{
    int ret;
    stroller_t *stlr = ptr;
    while (!is_stop) {
        pthread_mutex_lock(&stlr->follow_mutex);
        switch (stlr->mode) {
            case STLR_MODE_AUTO:
                // todo: Auto control
                {
                    static int cnt = 0;
                    if (++cnt % 1000) {
                        uint16_t distance_f;
                        uint8_t motion, distance_l, distance_r;
                        ret = sensor_config(stlr->sensor.hlk2411s, SENSOR_START_MEASURE, 0);
                        ret |= sensor_config(stlr->sensor.vl6180_l, SENSOR_START_MEASURE, 0);
                        ret |= sensor_config(stlr->sensor.vl6180_r, SENSOR_START_MEASURE, 0);
                        usleep(10);
                        if (!ret) {
                            ret = sensor_read(stlr->sensor.hlk2411s, &motion, SENSOR_CHANNEL0);
                            ret |= sensor_read(stlr->sensor.hlk2411s, &distance_f, SENSOR_CHANNEL1);
                            ret |= sensor_read(stlr->sensor.vl6180_l, &distance_l, SENSOR_CHANNEL0);
                            ret |= sensor_read(stlr->sensor.vl6180_r, &distance_r, SENSOR_CHANNEL0);
                        }
                        if (!ret) {
                            log_debug("motion:%d, distance_f:%d, distance_l:%d, distance_r:%d", motion, distance_f, distance_l, distance_r);
                        }
                    }
                }
                pthread_mutex_unlock(&stlr->follow_mutex);
                usleep(500 * 1000);
                break;
            case STLR_MODE_REMOTE:
                ret = move_of_remote(stlr, stlr->follow_cmd, stlr->follow_data[0], stlr->follow_data[1]);
                if (ret < 0) {
                    log_error("Invalid remote command cmd:%d, data1:%d, data2:%d", stlr->follow_cmd, stlr->follow_data[0], stlr->follow_data[1]);
                }
                pthread_cond_wait(&stlr->follow_cond, &stlr->follow_mutex);
                pthread_mutex_unlock(&stlr->follow_mutex);
                break;
            case STLR_MODE_NONE:
                pthread_cond_wait(&stlr->follow_cond, &stlr->follow_mutex);
                pthread_mutex_unlock(&stlr->follow_mutex);
                break;
            default:
                log_error("not find chassis mode %d", stlr->mode);
                pthread_cond_wait(&stlr->follow_cond, &stlr->follow_mutex);
                pthread_mutex_unlock(&stlr->follow_mutex);
                break;
        }
    }
    return NULL;
}

// Timing: y%d(cmd) %d(data1) %d(data2) %d(checksum)c
// todo: add checksum
static void comm_receive_callback(void *tcps)
{
    int ret = 0;
    stroller_t *stlr = container_of(tcps, stroller_t, comm.tcps);
    stlr_comm_t *comm = &stlr->comm;
    chassis_t *chassis = &stlr->chassis;
    int len = strlen(comm->rbuf);
    log_debug("rec[%d]: %s", len, comm->rbuf);
    if (len == 0 || comm->rbuf[0] != 'y' || comm->rbuf[len - 1] != 'c') {
        log_error("comm invalid data \"%s\"", comm->rbuf);
        goto clear;
    }

    char *p = comm->rbuf + 1;
    int cmd = strtol(p, &p, 10);
    int data1 = strtol(p, &p, 10);
    int data2 = strtol(p, &p, 10);
    int checksum = strtol(p, &p, 10);
    if (cmd + data1 + data2 != checksum) {
        log_error("comm checksum error");
        goto clear;
    }
    switch (cmd) {
        case COMM_CMD_STOP:
        case COMM_CMD_SPEED:
        case COMM_CMD_DIRECTION:
            pthread_mutex_lock(&stlr->follow_mutex);
            stlr->follow_cmd = cmd;
            stlr->follow_data[0] = data1;
            stlr->follow_data[1] = data2;
            pthread_cond_signal(&stlr->follow_cond);
            pthread_mutex_unlock(&stlr->follow_mutex);
            log_debug("comm remote, cmd:%d, data:%d %d", cmd, data1, data2);
            break;
        case COMM_CMD_ACTION_MODE:
            pthread_mutex_lock(&stlr->follow_mutex);
            stlr->mode = (data1 == 0) ? STLR_MODE_AUTO : STLR_MODE_REMOTE;
            stlr->follow_cmd = COMM_CMD_STOP;
            pthread_cond_signal(&stlr->follow_cond);
            pthread_mutex_unlock(&stlr->follow_mutex);
            log_debug("comm auto, ret:%d, data:%d", ret, data1);
            stlr_comm_send(stlr, "ok\n", 3);
            break;
        default:
            log_error("comm invalid command %d", cmd);
            break;
    }

clear:
    memset(comm->rbuf, 0, sizeof(comm->rbuf));
}

static void *comm_loop(void *ptr)
{
    int ret = 0;
    stroller_t *stlr = ptr;
    stlr_sensor_t *sensor = &stlr->sensor;
    stlr_comm_t *comm = &stlr->comm;
    tcp_server_t *tcps = &comm->tcps;
    char *sendbuf[100];
start_listen:
    ret = tcp_wait_for_client(tcps, 0);
    if (ret < 0) {
        perror("accept");
    }
    log_info("accept client connect");
    while (!is_stop) {
        /* sync read, it will callback when read success in the current thread. */
        ret = tcp_read_timeout(tcps, comm->rbuf, sizeof(comm->rbuf), 100);
        if (ret == 0) {
            log_info("Close connection from client");
            goto start_listen;
        }
        else if (ret == TIMEOUT_RET) {
            /* Not something to do */
        }
        else if (ret < 0) {
            break;
        }

        pthread_mutex_lock(&comm->send_lock);
        int len = strlen(comm->sbuf);
        if (len) {
            tcp_write(tcps, comm->sbuf, len);
            memset(comm->sbuf, 0, sizeof(comm->sbuf));
        }
        pthread_mutex_unlock(&comm->send_lock);
        // todo: tcp send buffer
    }
    tcp_stop_server(tcps);
    return NULL;
}

int stlr_start(stroller_t *stlr)
{
    int ret = 0;

    stlr->mode = STLR_MODE_NONE;

    ret = pthread_create(&stlr->sensor_thread, NULL, sensor_loop, stlr);
    if (ret) {
        log_error("Failed to create sensor thread");
        goto err;
    }
    log_info("stlr sensor thread created");

    ret = pthread_create(&stlr->comm_thread, NULL, comm_loop, stlr);
    if (ret) {
        log_error("Failed to create comm thread");
        goto err;
    }
    log_info("stlr comm thread created");

    ret = pthread_create(&stlr->follow_thread, NULL, stlr_follow_loop, stlr);
    if (ret) {
        log_error("Failed to create follow thread");
        goto err;
    }
    log_info("stlr follow thread created");

    return 0;

err:
    return -1;
}

int stlr_sensor_create(stroller_t *stlr)
{
    stlr->sensor.aht10 = sensor_create_with_register(SENSOR_TYPE_AHT10, stlr->iic);
    if (stlr->sensor.aht10 == NULL) {
        log_warn("Failed to create aht10, use fake sensor instead.");
        stlr->sensor.aht10 = sensor_create_with_register(SENSOR_TYPE_FAKE, NULL);
    }
    else {
        log_info("stlr sensor aht10 created");
    }

    stlr->sensor.mq135 = sensor_create_with_register(SENSOR_TYPE_MQ135, NULL);
    if (stlr->sensor.mq135 == NULL) {
        log_warn("Failed to create mq135, use fake sensor instead.");
        stlr->sensor.mq135 = sensor_create_with_register(SENSOR_TYPE_FAKE, NULL);
    }
    else {
        log_info("stlr sensor mq135 created");
    }

    stlr->sensor.sw18015 = sensor_create_with_register(SENSOR_TYPE_SW18015, NULL);
    if (stlr->sensor.sw18015 == NULL) {
        log_warn("Failed to create sw18015, use fake sensor instead.");
        stlr->sensor.sw18015 = sensor_create_with_register(SENSOR_TYPE_FAKE, NULL);
    }
    else {
        log_info("stlr sensor sw18015 created");
    }

    /* It is necessary to create a sensor for the distance of the 
    front, left and right sides. */
    stlr->sensor.hlk2411s = sensor_create_with_register(SENSOR_TYPE_HLK2411S, NULL);
    if (stlr->sensor.hlk2411s == NULL) {
        log_warn("Failed to create hlk2411s, use fake sensor instead.");
        stlr->sensor.hlk2411s = sensor_create_with_register(SENSOR_TYPE_FAKE, NULL);
    }
    else {
        log_info("stlr sensor hlk2411s created");
    }

    stlr->sensor.vl6180_l = sensor_create_with_register(SENSOR_TYPE_VL6180_1, stlr->iic);
    if (stlr->sensor.vl6180_l == NULL) {
        log_warn("Failed to create vl6180_l, use fake sensor instead.");
        stlr->sensor.vl6180_l = sensor_create_with_register(SENSOR_TYPE_FAKE, NULL);
    }
    else {
        log_info("stlr sensor vl6180_l created");
    }

    stlr->sensor.vl6180_r = sensor_create_with_register(SENSOR_TYPE_VL6180_2, stlr->iic);
    if (stlr->sensor.vl6180_r == NULL) {
        log_warn("Failed to create vl6180_r, use fake sensor instead.");
        stlr->sensor.vl6180_r = sensor_create_with_register(SENSOR_TYPE_FAKE, NULL);
    }
    else {
        log_info("stlr sensor vl6180_r created");
    }

    pthread_mutex_init(&stlr->sensor.mutex, NULL);
    return 0;
}

int stlr_sensor_destroy(stroller_t *stlr)
{
    sensor_destroy(stlr->sensor.aht10);
    sensor_destroy(stlr->sensor.mq135);
    sensor_destroy(stlr->sensor.sw18015);
    sensor_destroy(stlr->sensor.hlk2411s);
    sensor_destroy(stlr->sensor.vl6180_l);
    sensor_destroy(stlr->sensor.vl6180_r);
    return 0;
}

stroller_t *stlr_create()
{
    int ret = 0;
    stroller_t *stlr;
    stlr = calloc(1, sizeof(stroller_t));
    if (stlr == NULL) {
        log_error("Failed to calloc");
        goto err0;
    }

    ret = chassis_register(&stlr->chassis);
    if (ret) {
        log_error("Failed to create chassis");
        goto err1;
    }
    set_chassis_speed(&stlr->chassis, 0);// stop
    set_chassis_turn(&stlr->chassis, 0); // straight
    if (get_chassis_speed(&stlr->chassis) != 0 || get_chassis_turn(&stlr->chassis) != 0) {
        log_error("Failed to set chassis speed and turn");
        goto err2;
    }

    stlr->iic = iic_create(USE_IIC_BUS);
    if (stlr->iic == NULL) {
        log_error("Failed to create iic");
        goto err3;
    }

    ret = stlr_sensor_create(stlr);
    if (ret) {
        log_warn("Failed to init sensor");
        goto err3;
    }

    // stlr->comm.comm = comm_create_with_register(COMM_TYPE_UART);
    // if (stlr->comm.comm == NULL) {
    //     log_error("Failed to create stlr");
    //     goto err3;
    // }
    ret = tcp_start_server(&stlr->comm.tcps, TCP_SERVER_IP, TCP_SERVER_PORT);
    if (ret != 0) {
        log_error("Failed to start server on port %d", TCP_SERVER_PORT);
    }
    else {
        log_info("tcp server started on port %d", TCP_SERVER_PORT);
    }
    tcp_set_client_handler(&stlr->comm.tcps, comm_receive_callback);
    pthread_mutex_init(&stlr->comm.send_lock, NULL);
    memset(stlr->comm.sbuf, 0, sizeof(stlr->comm.sbuf));

    pthread_mutex_init(&stlr->follow_mutex, NULL);
    pthread_cond_init(&stlr->follow_cond, NULL);
    return stlr;

err3:
    stlr_sensor_destroy(stlr);
    iic_destroy(stlr->iic);
err2:
    chassis_destroy(&stlr->chassis);
err1:
    free(stlr);
err0:
    return NULL;
}

void stlr_destroy(stroller_t *stlr)
{
    comm_destroy(stlr->comm.comm);

    stlr_sensor_destroy(stlr);

    chassis_destroy(&stlr->chassis);

    free(stlr);
}

void stlr_stop()
{
    is_stop = 1;
}