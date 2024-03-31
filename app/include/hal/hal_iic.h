#ifndef __HAL_IIC_H__
#define __HAL_IIC_H__

#include <stdint.h>
#include <pthread.h>
typedef struct iic_dev {
    int fd;
    int bus;
    pthread_mutex_t lock;
} iic_dev_t;

int iic_write(iic_dev_t *iic, uint8_t addr, const uint8_t *buf, int len);
int iic_read(iic_dev_t *iic, uint8_t addr, uint8_t *buf, int len);
int iic_reg8_write(iic_dev_t *iic, uint8_t addr, uint8_t reg, uint8_t *buf, int len);
int iic_reg8_read(iic_dev_t *iic, uint8_t addr, uint8_t reg, uint8_t *buf, int len);
int iic_reg16_write(iic_dev_t *iic, uint8_t addr, uint16_t reg, uint8_t *buf, int len);
int iic_reg16_read(iic_dev_t *iic, uint8_t addr, uint16_t reg, uint8_t *buf, int len);
iic_dev_t *iic_create(int bus);
void iic_destroy(iic_dev_t *iic);


#endif