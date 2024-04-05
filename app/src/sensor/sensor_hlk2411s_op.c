#include <bits/pthreadtypes.h>
#include <stdlib.h>
#include "log.h"
#include "sensor.h"
#include "sensor_op.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <termios.h>
#include <unistd.h>

typedef struct hlk2411s_data {
    int fd;
    uint8_t motion;
    uint16_t distance;
    pthread_mutex_t lock;
    pthread_t thread;
} hlk2411s_data_t;

void hlk2411_loop(hlk2411s_data_t *data)
{
    char buffer[20];
    while (1) {
        // 清除缓冲区
        int bytes_read = read(data->fd, buffer, sizeof(buffer));
        if (bytes_read > 0) {
            if (buffer[0] != 0xAA || buffer[1] != 0xAA || buffer[5] != 0x55 || buffer[6] != 0x55) {
                log_warn("Failed to decode of hlk2411s\n");
                continue;
            }
            // pthread_mutex_lock(&data->lock);
            data->motion = buffer[2];
            data->distance = (buffer[4] << 8 | buffer[3]);
            // pthread_mutex_unlock(&data->lock);
        }
        usleep(100);
    }
}

static int hlk2411s_init(sensor_t *sensor)
{
    hlk2411s_data_t *data;
    data = calloc(1, sizeof(hlk2411s_data_t));

    // Open the serial port
    data->fd = open("/dev/ttyS5", O_RDWR | O_NOCTTY | O_NDELAY);
    if (data->fd == -1) {
        log_error("Unable to open serial port");
        return -1;
    }

    // Configure the serial port
    struct termios options;// Structure to hold the serial port configuration options

    tcgetattr(data->fd, &options);// Get the current attributes of the serial port

    cfsetispeed(&options, 115200);                     // Set the input baud rate to 115200
    cfsetospeed(&options, 115200);                     // Set the output baud rate to 115200
    options.c_cflag |= (CLOCAL | CREAD);               // Enable receiver and set local mode
    options.c_cflag &= ~PARENB;                        // Disable parity bit
    options.c_cflag &= ~CSTOPB;                        // Set one stop bit
    options.c_cflag &= ~CSIZE;                         // Clear the data size bits
    options.c_cflag |= CS8;                            // Set data size to 8 bits
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);// Disable canonical mode, echo, erase, and kill processing
    options.c_iflag &= ~(IXON | IXOFF | IXANY);        // Disable software flow control
    options.c_oflag &= ~OPOST;                         // Disable output processing
    tcsetattr(data->fd, TCSANOW, &options);            // Set the new attributes of the serial port immediately

    // pthread_mutex_init(&data->lock, NULL);
    pthread_create(&data->thread, NULL, (void *)hlk2411_loop, data);

    sensor->priv = data;
    return 0;
}

static int hlk2411s_deinit(sensor_t *sensor)
{
    struct hlk2411s_data *data = sensor->priv;
    // pthread_mutex_destroy(&data->lock);
    pthread_cancel(data->thread);
    close(data->fd);
    free(sensor->priv);
    return 0;
}

static int hlk2411s_read(sensor_t *sensor, void *value, int channel)
{
    struct hlk2411s_data *data = sensor->priv;
    // pthread_mutex_lock(&data->lock);
    switch (channel) {
        case SENSOR_CHANNEL0:
            *(uint8_t *)value = data->motion;
            break;
        case SENSOR_CHANNEL1:
            *(uint16_t *)value = data->distance;
            break;
    }
    // pthread_mutex_unlock(&data->lock);
    return 0;
}

static int hlk2411s_config(sensor_t *sensor, int cmd, unsigned long arg)
{
    return 0;
}

static const sensor_op_t hlk2411s_op = {
    .init = hlk2411s_init,
    .deinit = hlk2411s_deinit,
    .read = hlk2411s_read,
    .config = hlk2411s_config,
};

int hlk2411s_sensor_register(sensor_t *sensor)
{
    sensor->op = &hlk2411s_op;
    return 0;
}