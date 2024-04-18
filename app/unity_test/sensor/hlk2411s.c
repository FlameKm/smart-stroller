#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include "log.h"
#include "sensor.h"
#include "sensor_platfrom.h"
#include <signal.h>

typedef struct hlk2411 {
    uint8_t motion;
    uint16_t distance;
} hlk2411_t;


bool is_stop = false;

void sig_handler(int signo)
{
    if (signo == SIGINT) {
        log_info("received SIGINT");
        is_stop = true;
    }
}

int nopack()
{
    int fd;
    char buffer[255];
    hlk2411_t data;
    // Open the serial port
    fd = open("/dev/ttyS5", O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("Unable to open serial port");
        return -1;
    }

    // Configure the serial port
    struct termios options;// Structure to hold the serial port configuration options

    tcgetattr(fd, &options);// Get the current attributes of the serial port

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
    tcsetattr(fd, TCSANOW, &options);                  // Set the new attributes of the serial port immediately

    // Read data from the serial port
    while (1) {
        // 清除缓冲区
        int bytes_read = read(fd, buffer, sizeof(buffer));
        if (bytes_read > 0) {
            // hex output
            for (int i = 0; i < bytes_read; i++) {
                printf("%02X ", buffer[i]);
            }
            if (buffer[0] != 0xAA || buffer[1] != 0xAA || buffer[5] != 0x55 || buffer[6] != 0x55) {
                log_warn("Failed to decode\n");
                continue;
            }
            data.motion = buffer[2];
            data.distance = (buffer[4] << 8 | buffer[3]);
            printf("motion: %d, distance: %d", data.motion, data.distance);
            printf("\n");
        }
        usleep(50 * 1000);
    }

    // Close the serial port
    close(fd);

    return 0;
}

int main()
{
    sensor_t *sensor;
    hlk2411_t data;
    signal(SIGINT, sig_handler);
    sensor = sensor_create_with_register(SENSOR_TYPE_HLK2411S, NULL);
    if (sensor == NULL) {
        log_error("Failed to create sensor\n");
        return -1;
    }
    while (!is_stop) {
        sensor_read(sensor, &data.motion, SENSOR_CHANNEL0);
        sensor_read(sensor, &data.distance, SENSOR_CHANNEL1);
        log_info("motion: %d, distance: %d", data.motion, data.distance);
        usleep(100 * 1000);
    }

    sensor_destroy(sensor);
    return 0;
}