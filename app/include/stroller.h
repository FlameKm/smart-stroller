#ifndef __STROLLER_H__
#define __STROLLER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "chassis.h"
#include "sensor.h"
#include "comm.h"
#include "hal_iic.h"
#include "tcpserver.h"

typedef struct stlr_sensor {
    // base sensor
    sensor_t *aht10;  // temperature and humidity
    sensor_t *mq135;  // air quality
    sensor_t *sw18015;// shake
    // smart control
    sensor_t *hlk2411s;// distance for front
    sensor_t *vl6180_l;// distance for left
    sensor_t *vl6180_r;// distance for right

    // data
    float temp;
    float humi;
    float co2;
    int shake;
    bool motion;
    float distance_f;
    float distance_l;
    float distance_r;

    bool available;
    pthread_mutex_t mutex;
} stlr_sensor_t;

typedef enum STLR_MODE
{
    STLR_MODE_NONE,
    STLR_MODE_AUTO,
    STLR_MODE_REMOTE,
} STLR_MODE;

typedef enum COMM_COMMAND
{
    COMM_CMD_NONE,
    COMM_CMD_STOP,
    COMM_CMD_SPEED,
    COMM_CMD_DIRECTION,
    COMM_CMD_ACTION_MODE,
} COMM_COMMAND;

typedef struct stlr_comm {
    comm_t *comm;
    tcp_server_t tcps;
    char rbuf[100];
    char sbuf[100];
    pthread_mutex_t send_lock;
} stlr_comm_t;

typedef struct stroller {
    stlr_sensor_t sensor;
    stlr_comm_t comm;
    iic_dev_t *iic;
    chassis_t chassis;

    enum STLR_MODE mode;
    pthread_cond_t follow_cond;
    pthread_mutex_t follow_mutex;

    pthread_t sensor_thread;
    pthread_t comm_thread;
    pthread_t follow_thread;

    int follow_cmd;
    int follow_data[2];
} stroller_t;


stroller_t *stlr_create();
void stlr_destroy(stroller_t *strl);
int stlr_start(stroller_t *strl);
void stlr_stop();

#ifdef __cplusplus
}
#endif

#endif