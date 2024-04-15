#include "stroller.h"
#include "log.h"
#include "stdlib.h"
#include "tools.h"
#include <string.h>
#include <unistd.h>

#define TCP_SERVER_PORT 8888
#define TCP_SERVER_IP "0.0.0.0"
#define USE_IIC_BUS 2

static int is_stop = 0;
static void *sensor_loop(void *ptr)
{
    int ret = 0;
    stroller_t *stlr = ptr;
    stlr_sensor_t *sensor = &stlr->sensor;
    float temp, humi, co2, shake;
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

        pthread_mutex_lock(&sensor->mutex);
        sensor->available = true;
        sensor->temp = temp;
        sensor->humi = humi;
        sensor->co2 = co2;
        sensor->shake = shake;
        pthread_mutex_unlock(&sensor->mutex);

        sleep(1);
    }
    return NULL;
}

static void *stlr_follow_loop(void *ptr)
{
    stroller_t *stlr = ptr;
    stlr_chassis_t *chassis = &stlr->chassis;
    while (!is_stop) {
        pthread_mutex_lock(&stlr->follow_mutex);
        switch (stlr->mode) {
            case STLR_MODE_AUTO:
                // todo: Auto control
                usleep(20 * 2000);
                break;
            case STLR_MODE_REMOTE:
                pthread_cond_wait(&stlr->follow_cond, &stlr->follow_mutex);
                break;
            default:
                log_error("not find chassic mode %d", stlr->mode);
                break;
        }
        pthread_mutex_unlock(&stlr->follow_mutex);
    }
    return NULL;
}

// Timing: 0xAA 0xBB cmd_h cmd_l data_h data_l 0x00 0x22
static void comm_receive_callback(void *tcps)
{
    int ret = 0;
    stroller_t *stlr = container_of(tcps, stroller_t, comm.tcps);
    stlr_comm_t *comm = &stlr->comm;
    stlr_chassis_t *chassis = &stlr->chassis;
    int len = strlen(comm->rbuf);
    log_info("rec[%d]: %s", len, comm->rbuf);

    if (len != 8) {
        log_error("Invalid data length %d", len);
        return;
    }
    int cmd = (comm->rbuf[2] << 8) | comm->rbuf[3];
    int data = (comm->rbuf[4] << 8) | comm->rbuf[5];
    switch (cmd) {
        case COMM_COMMAND_STOP:
            log_debug("comm stop, ret:%d, data:%d", ret, data);
            break;
        case COMM_COMMAND_FORWARD:
            log_debug("comm forward, ret:%d, data:%d", ret, data);
            break;
        case COMM_COMMAND_BACKWARD:
            log_debug("comm backward, ret:%d, data:%d", ret, data);
            break;
        case COMM_COMMAND_LEFT:
            log_debug("comm left, ret:%d, data:%d", ret, data);
            break;
        case COMM_COMMAND_RIGHT:
            log_debug("comm right, ret:%d, data:%d", ret, data);
            break;
        case COMM_COMMAND_ACTION_MODE:
            pthread_mutex_lock(&stlr->follow_mutex);
            stlr->mode = data == 0 ? STLR_MODE_AUTO : STLR_MODE_REMOTE;
            pthread_cond_signal(&stlr->follow_cond);
            pthread_mutex_unlock(&stlr->follow_mutex);
            log_debug("comm auto, ret:%d, data:%d", ret, data);
            break;
        default:
            log_error("comm invalid command %d", cmd);
            break;
    }

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
    log_debug("accept client connect");
    while (!is_stop) {
        // sync read, it will callback when read success.
        ret = tcp_read_timeout(tcps, comm->rbuf, sizeof(comm->rbuf), 100);
        if (ret == 0) {
            log_debug("Close connection from client");
            goto start_listen;
        }
        else if (ret == TIMEOUT_RET) {
            // continue;
        }
        else if (ret < 0) {
            break;
        }

        pthread_mutex_lock(&sensor->mutex);
        if (sensor->available) {
            // todo: pack sensor data to sendbuf
            sensor->available = false;
        }
        pthread_mutex_unlock(&sensor->mutex);
        tcp_write(tcps, comm->sbuf, strlen(comm->sbuf));
    }
    tcp_stop_server(tcps);
    return NULL;
}

int stlr_start_loop(stroller_t *stlr)
{
    int ret = 0;

    ret = pthread_create(&stlr->sensor_thread, NULL, sensor_loop, stlr);
    if (ret) {
        log_error("Failed to create sensor thread");
        goto err;
    }

    ret = pthread_create(&stlr->comm_thread, NULL, comm_loop, stlr);
    if (ret) {
        log_error("Failed to create comm thread");
        goto err;
    }

    ret = pthread_create(&stlr->follow_thread, NULL, stlr_follow_loop, stlr);
    if (ret) {
        log_error("Failed to create follow thread");
        goto err;
    }

    return 0;

err:
    return -1;
}

int stlr_sensor_create(stroller_t *stlr)
{
    stlr->sensor.aht10 = sensor_create_with_register(SENSOR_TYPE_AHT10, stlr->iic);
    if (stlr->sensor.aht10 == NULL) {
        log_warn("Failed to create aht10");
        stlr->sensor.aht10 = sensor_create_with_register(SENSOR_TYPE_FAKE, NULL);
    }

    stlr->sensor.mq135 = sensor_create_with_register(SENSOR_TYPE_MQ135, NULL);
    if (stlr->sensor.mq135 == NULL) {
        log_warn("Failed to create mq135");
        stlr->sensor.mq135 = sensor_create_with_register(SENSOR_TYPE_FAKE, NULL);
    }

    stlr->sensor.sw18015 = sensor_create_with_register(SENSOR_TYPE_SW18015, NULL);
    if (stlr->sensor.sw18015 == NULL) {
        log_warn("Failed to create sw18015");
        stlr->sensor.sw18015 = sensor_create_with_register(SENSOR_TYPE_FAKE, NULL);
    }

    /* It is necessary to create a sensor for the distance of the 
    front, left and right sides. */
    stlr->sensor.hlk2411s = sensor_create_with_register(SENSOR_TYPE_HLK2411S, NULL);
    if (stlr->sensor.hlk2411s == NULL) {
        log_error("Failed to create hlk2411s");
        return -1;
    }

    stlr->sensor.vl6180_l = sensor_create_with_register(SENSOR_TYPE_VL6180_1, stlr->iic);
    if (stlr->sensor.vl6180_l == NULL) {
        log_error("Failed to create vl6180_l");
        return -1;
    }

    stlr->sensor.vl6180_r = sensor_create_with_register(SENSOR_TYPE_VL6180_2, stlr->iic);
    if (stlr->sensor.vl6180_r == NULL) {
        log_error("Failed to create vl6180_r");
        return -1;
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

    stlr->chassis.mr = motor_create(MOTOR_CURRENT_OPEN, 1);
    stlr->chassis.ml = motor_create(MOTOR_CURRENT_OPEN, 2);
    stlr->chassis.servo = servo_create();
    if (stlr->chassis.mr == NULL || stlr->chassis.ml == NULL || stlr->chassis.servo == NULL) {
        log_error("Failed to create chassis");
        goto err1;
    }

    stlr->iic = iic_create(USE_IIC_BUS);
    ret = stlr_sensor_create(stlr);
    if (ret) {
        log_error("Failed to init sensor");
        goto err2;
    }

    // stlr->comm.comm = comm_create_with_register(COMM_TYPE_UART);
    // if (stlr->comm.comm == NULL) {
    //     log_error("Failed to create stlr");
    //     goto err3;
    // }
    ret = tcp_start_server(&stlr->comm.tcps, TCP_SERVER_IP, TCP_SERVER_PORT);
    if (ret != 0) {
        log_warn("Failed to start server on port %d", TCP_SERVER_PORT);
    }
    tcp_set_client_handler(&stlr->comm.tcps, comm_receive_callback);

    pthread_mutex_init(&stlr->follow_mutex, NULL);
    pthread_cond_init(&stlr->follow_cond, NULL);
    return stlr;

err3:
err2:
    stlr_sensor_destroy(stlr);
    motor_destroy(stlr->chassis.mr);
    motor_destroy(stlr->chassis.ml);
    servo_destroy(stlr->chassis.servo);
err1:
    free(stlr);
err0:
    return NULL;
}

void stlr_destroy(stroller_t *stlr)
{
    comm_destroy(stlr->comm.comm);

    stlr_sensor_destroy(stlr);

    motor_destroy(stlr->chassis.mr);
    motor_destroy(stlr->chassis.ml);
    servo_destroy(stlr->chassis.servo);

    free(stlr);
}

void stlr_stop()
{
    is_stop = 1;
}