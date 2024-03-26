#include "linux/cdev.h"
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/mman.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/backing-dev.h>
#include <linux/shmem_fs.h>
#include <linux/splice.h>
#include <linux/pfn.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/uio.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/pwm.h>

#define DEVICE_NAME "yc-motor"

#define PWMLED_PERIOD 1000000// 脉冲周期固定为1ms

#define MOTOR_SET_SPEED 0
#define MOTOR_GET_SPEED 1
#define MOTOR_START 2
#define MOTOR_STOP 3
struct motor_data {
    uint32_t pwm_ch;
    struct pwm_device *pwm;
    int speed;

    dev_t dev;
    struct cdev cdev;
    struct device *device;
    struct class *class;
};

static int motor_open(struct inode *node, struct file *file)
{
    struct motor_data *data = container_of(node->i_cdev, struct motor_data, cdev);
    file->private_data = data;
    return 0;
}

static int motor_release(struct inode *node, struct file *file)
{
    file->private_data = NULL;
    return 0;
}
static long motor_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    struct motor_data *data = file->private_data;

    switch (cmd) {
        case MOTOR_SET_SPEED: {
            uint32_t duty_us;
            ret = (int)copy_from_user((void *)&duty_us, (void *)arg, sizeof(duty_us));
            pwm_config(data->pwm, duty_us * 1000, PWMLED_PERIOD);
            break;
        }
        case MOTOR_GET_SPEED: {
            break;
        }
        case MOTOR_START: {
            pwm_enable(data->pwm);
            break;
        }
        case MOTOR_STOP: {
            pwm_disable(data->pwm);
            break;
        }
        default:
            break;
    }

    return ret;
}
static const struct file_operations motor_drv_fops = {
    .owner = THIS_MODULE,
    .open = motor_open,
    .unlocked_ioctl = motor_ioctl,
    .release = motor_release,
};

static int motor_chrdev_create(struct platform_device *pdev, struct motor_data *data)
{
    int ret = 0;
    ret = alloc_chrdev_region(&data->dev, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        dev_err(&pdev->dev, "Failed to allocate character device region\n");
        goto err1;
    }
    cdev_init(&data->cdev, &motor_drv_fops);
    ret = cdev_add(&data->cdev, data->dev, 1);
    if (ret < 0) {
        dev_err(&pdev->dev, "Failed to add character device\n");
        goto err2;
    }
    data->class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(data->class)) {
        dev_err(&pdev->dev, "Failed to create device class\n");
        goto err3;
    }

    data->device = device_create(data->class, NULL, data->dev, NULL, "motor");
    if (IS_ERR(data->device)) {
        dev_err(&pdev->dev, "Failed to create device\n");
        goto err4;
    }
    return 0;

err4:
    class_destroy(data->class);
err3:
    cdev_del(&data->cdev);
err2:
    unregister_chrdev_region(data->dev, 1);
err1:
    return -1;
}

static int motor_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct device_node *node = pdev->dev.of_node;
    struct motor_data *data = NULL;

    data = devm_kmalloc(&pdev->dev, sizeof(struct motor_data), GFP_KERNEL);
    if (!data) {
        dev_err(&pdev->dev, "Failed to allocate memory\n");
        goto err0;
    }

    ret = of_property_read_u32(node, "pwm", &data->pwm_ch);
    if (ret < 0) {
        dev_err(&pdev->dev, "Failed to of property read 'pwm'\n");
        goto err1;
    }
    data->pwm = pwm_request(data->pwm_ch, "motor");

    ret = motor_chrdev_create(pdev, data);
    if (ret < 0) {
        dev_err(&pdev->dev, "Failed to chrdev create\n");
        goto err2;
    }

    platform_set_drvdata(pdev, data);

    return 0;

err2:
    pwm_disable(data->pwm);
    pwm_free(data->pwm);
err1:
    devm_kfree(&pdev->dev, data);
err0:
    return -1;
}

static int motor_remove(struct platform_device *pdev)
{
    struct motor_data *data = platform_get_drvdata(pdev);

    pwm_disable(data->pwm);
    pwm_free(data->pwm);

    device_destroy(data->class, data->dev);
    class_destroy(data->class);
    cdev_del(&data->cdev);
    unregister_chrdev_region(data->dev, 1);

    return 0;
}


static const struct of_device_id motor_id[] = {
    {.compatible = "yc,motor"},
    {},
};

static struct platform_driver chip_motor_driver = {
    .probe = motor_probe,
    .remove = motor_remove,
    .driver = {
        .name = "yc_motor",
        .of_match_table = motor_id,
    },
};

static int motor_init(void)
{
    return platform_driver_register(&chip_motor_driver);
}


static void motor_exit(void)
{
    platform_driver_unregister(&chip_motor_driver);
}


module_init(motor_init);
module_exit(motor_exit);
MODULE_LICENSE("GPL");