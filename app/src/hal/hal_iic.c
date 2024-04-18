#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "hal_iic.h"

iic_dev_t *iic_create(int bus)
{
    iic_dev_t *iic;
    char dev_name[11];
    int fd;
    iic = calloc(1, sizeof(iic_dev_t));
    if (iic == NULL) {
        return NULL;
    }

    snprintf(dev_name, 11, "/dev/i2c-%d", bus);
    fd = open(dev_name, O_RDWR);
    if (fd < 0) {
        return NULL;
    }

    pthread_mutex_init(&iic->lock, NULL);
    iic->bus = bus;
    iic->fd = fd;
    return iic;
}


void iic_destroy(iic_dev_t *iic)
{
    if (iic == NULL) {
        return;
    }
    pthread_mutex_destroy(&iic->lock);
    close(iic->fd);
    free(iic);
}

int iic_write(iic_dev_t *iic, uint8_t addr, const uint8_t *buf, int len)
{
    int ret = 0;
    pthread_mutex_lock(&iic->lock);
    ret = ioctl(iic->fd, I2C_SLAVE_FORCE, addr);
    if (ret < 0) {
        ret = -1;
        goto err;
    }
    ret = write(iic->fd, buf, len);
    if (ret < 0) {
        ret = -2;
        goto err;
    }
err:
    pthread_mutex_unlock(&iic->lock);
    return ret;
}

int iic_read(iic_dev_t *iic, uint8_t addr, uint8_t *buf, int len)
{
    int ret;

    pthread_mutex_lock(&iic->lock);
    ret = ioctl(iic->fd, I2C_SLAVE, addr);
    if (ret < 0) {
        ret = -1;
        goto err;
    }
    ret = read(iic->fd, buf, len);
    if (ret < 0) {
        ret = -2;
        goto err;
    }
err:
    pthread_mutex_unlock(&iic->lock);
    return 0;
}

int iic_reg8_write(iic_dev_t *iic, uint8_t addr, uint8_t reg, uint8_t *buf, int len)
{
    int ret;
    struct i2c_rdwr_ioctl_data data;
    unsigned char sendbuf[20] = {0};

    sendbuf[0] = (reg >> 8) & 0xffu;
    for (int i = 0; i < len; i++) {
        sendbuf[1 + i] = buf[i];
    }

    data.msgs = (struct i2c_msg *)calloc(1, sizeof(struct i2c_msg));
    data.nmsgs = 1;

    data.msgs[0].len = 1 + len;
    data.msgs[0].addr = addr;
    data.msgs[0].flags = 0;
    data.msgs[0].buf = sendbuf;

    pthread_mutex_lock(&iic->lock);
    ret = ioctl(iic->fd, I2C_RDWR, (unsigned long)&data);
    pthread_mutex_unlock(&iic->lock);
    return 0;
}

int iic_reg8_read(iic_dev_t *iic, uint8_t addr, uint8_t reg, uint8_t *buf, int len)
{
    int ret = 0;
    struct i2c_rdwr_ioctl_data data;
    unsigned char sendbuf[12] = {0};
    sendbuf[0] = reg & 0xffu;

    data.msgs = (struct i2c_msg *)calloc(2, sizeof(struct i2c_msg));
    data.nmsgs = 2;

    data.msgs[0].len = 1;
    data.msgs[0].addr = addr;
    data.msgs[0].flags = 0;
    data.msgs[0].buf = sendbuf;

    data.msgs[1].len = (uint8_t)len;
    data.msgs[1].addr = addr;
    data.msgs[1].flags = 1;
    data.msgs[1].buf = buf;

    pthread_mutex_lock(&iic->lock);
    ret = ioctl(iic->fd, I2C_RDWR, (unsigned long)&data);
    pthread_mutex_unlock(&iic->lock);

    free(data.msgs);
    return ret;
}

int iic_reg16_write(iic_dev_t *iic, uint8_t addr, uint16_t reg, uint8_t *buf, int len)
{
    int ret = 0;
    struct i2c_rdwr_ioctl_data data;
    unsigned char sendbuf[20] = {0};

    sendbuf[0] = (reg >> 8) & 0xffu;
    sendbuf[1] = reg & 0xffu;
    for (int i = 0; i < len; i++) {
        sendbuf[2 + i] = buf[i];
    }

    data.msgs = (struct i2c_msg *)calloc(1, sizeof(struct i2c_msg));
    data.nmsgs = 1;

    data.msgs[0].len = 2 + len;
    data.msgs[0].addr = addr;
    data.msgs[0].flags = 0;
    data.msgs[0].buf = sendbuf;

    pthread_mutex_lock(&iic->lock);
    ret = ioctl(iic->fd, I2C_RDWR, (unsigned long)&data);
    pthread_mutex_unlock(&iic->lock);
    return 0;
}

int iic_reg16_read(iic_dev_t *iic, uint8_t addr, uint16_t reg, uint8_t *buf, int len)
{
    int ret = 0;
    struct i2c_rdwr_ioctl_data data;
    unsigned char sendbuf[12] = {0};
    sendbuf[0] = (reg >> 8) & 0xffu;
    sendbuf[1] = reg & 0xffu;

    data.msgs = (struct i2c_msg *)calloc(2, sizeof(struct i2c_msg));
    data.nmsgs = 2;

    data.msgs[0].len = 2;
    data.msgs[0].addr = addr;
    data.msgs[0].flags = 0;
    data.msgs[0].buf = sendbuf;

    data.msgs[1].len = (uint8_t)len;
    data.msgs[1].addr = addr;
    data.msgs[1].flags = 1;
    data.msgs[1].buf = buf;

    pthread_mutex_lock(&iic->lock);
    ret = ioctl(iic->fd, I2C_RDWR, (unsigned long)&data);
    pthread_mutex_unlock(&iic->lock);

    free(data.msgs);
    return 0;
}
