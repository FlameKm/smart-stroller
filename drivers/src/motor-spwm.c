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
#include <linux/gpio.h>
#include <linux/hrtimer.h>

#define DEVICE_NAME "yc-motor"

#define MOTOR_SET_SPEED _IO('M', 0)//uint32_t
#define MOTOR_GET_SPEED _IO('M', 1)
#define MOTOR_START _IO('M', 2)
#define MOTOR_STOP _IO('M', 3)

enum PWM_STATE
{
    PWM_DISABLE = 0,
    PWM_START,
    PWM_RISING,
    PWM_FALLING,
};

struct motor_data {
    uint32_t pwm_ch;
    uint32_t duty_us;
    uint32_t period_us;
    int gpio;
    struct hrtimer timer;
    uint16_t status;
    struct device *device;
};

struct motor_dev {
    struct motor_data *data;
    int num;

    dev_t dev;
    struct cdev cdev;
    struct class *class;
};

static int start_pwm_hrtimer(struct motor_data *data, enum PWM_STATE state, uint32_t delay_us)
{
    ktime_t ktime;
    data->status = state;
    ktime = ktime_set(0, delay_us * 1000);
    hrtimer_start(&data->timer, ktime, HRTIMER_MODE_REL);
    return 0;
}

enum hrtimer_restart pwm_hrtimer_handler(struct hrtimer *hrtimer)
{
    struct motor_data *data = container_of(hrtimer, struct motor_data, timer);
    switch (data->status) {
        case PWM_START:
        case PWM_RISING:
            gpio_set_value(data->gpio, 1);
            start_pwm_hrtimer(data, PWM_FALLING, data->duty_us);
            break;
        case PWM_FALLING:
            gpio_set_value(data->gpio, 0);
            start_pwm_hrtimer(data, PWM_RISING, data->period_us - data->duty_us);
            break;
        case PWM_DISABLE:
            gpio_set_value(data->gpio, 0);
            return HRTIMER_NORESTART;
        default:
            break;
    }
    return HRTIMER_NORESTART;
}

static int motor_open(struct inode *node, struct file *file)
{
    struct motor_dev *dev = container_of(node->i_cdev, struct motor_dev, cdev);
    struct inode *inode = file_inode(file);
    file->private_data = &dev->data[iminor(inode)];
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
            ret = (int)copy_from_user((void *)&data->duty_us, (void *)arg, sizeof(data->duty_us));
            break;
        }
        case MOTOR_GET_SPEED: {
            ret = (int)copy_to_user((void *)arg, &data->duty_us, sizeof(data->duty_us));
            break;
        }
        case MOTOR_START: {
            start_pwm_hrtimer(data, PWM_START, 1000);
            break;
        }
        case MOTOR_STOP: {
            start_pwm_hrtimer(data, PWM_DISABLE, 1000);
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

static int motor_chrdev_create(struct platform_device *pdev, struct motor_dev *dev, int num)
{
    int ret = 0;
    ret = alloc_chrdev_region(&dev->dev, 0, num, DEVICE_NAME);
    if (ret < 0) {
        dev_err(&pdev->dev, "Failed to allocate character device region\n");
        goto err1;
    }
    cdev_init(&dev->cdev, &motor_drv_fops);
    ret = cdev_add(&dev->cdev, dev->dev, num);
    if (ret < 0) {
        dev_err(&pdev->dev, "Failed to add character device\n");
        goto err2;
    }
    dev->class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(dev->class)) {
        dev_err(&pdev->dev, "Failed to create device class\n");
        goto err3;
    }
    for (int i = 0; i < num; i++) {
        dev->data[i].device = device_create(dev->class, NULL, dev->dev + i, NULL, "motor%d", i);
        if (IS_ERR(dev->data[i].device)) {
            dev_err(&pdev->dev, "Failed to create device %d\n", MINOR(dev->dev + i));
            goto err4;
        }
    }
    return 0;

err4:
    class_destroy(dev->class);
err3:
    cdev_del(&dev->cdev);
err2:
    unregister_chrdev_region(dev->dev, num);
err1:
    return -1;
}

static int pwm_to_gpio(int pwm)
{
    int gpio_pin = 0;
    switch (pwm) {
        case 1: gpio_pin = 231; break;
        case 2: gpio_pin = 232; break;
        default: return 0;
    }
    return gpio_pin;
}

static int motor_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct device_node *node = pdev->dev.of_node;
    struct motor_dev *dev = NULL;
    struct motor_data *data;
    int num = 0;

    dev = devm_kmalloc(&pdev->dev, sizeof(struct motor_dev), GFP_KERNEL);
    if (!dev) {
        dev_err(&pdev->dev, "Failed to allocate memory\n");
        goto err0;
    }

    num = of_property_count_elems_of_size(node, "motor-pwm", 12);
    dev->num = num;
    data = kzalloc(sizeof(struct motor_data) * num, GFP_KERNEL);
    if (IS_ERR(data)) {
        goto err1;
    }
    dev->data = data;

    ret = motor_chrdev_create(pdev, dev, num);
    if (ret < 0) {
        dev_err(&pdev->dev, "Failed to chrdev create\n");
        goto err2;
    }

    for (int i = 0; i < num; i++) {
        ret = of_property_read_u32_index(node, "motor-pwm", 0 + i * 3, &data[i].pwm_ch);
        ret |= of_property_read_u32_index(node, "motor-pwm", 1 + i * 3, &data[i].duty_us);
        ret |= of_property_read_u32_index(node, "motor-pwm", 2 + i * 3, &data[i].period_us);
        if (ret < 0) {
            dev_err(&pdev->dev, "Failed to of property read 'motor-pwm'\n");
            goto err3;
        }
        dev_info(&pdev->dev, "pwm %d, duty %d, period %d\n", data[i].pwm_ch, data[i].duty_us, data[i].period_us);
        data[i].gpio = pwm_to_gpio(data[i].pwm_ch);
        if (!data->gpio) {
            dev_err(&pdev->dev, "Failed to get gpio\n");
            goto err3;
        }
        data[i].status = PWM_DISABLE;
        gpio_request(data[i].gpio, "gpio");
        gpio_direction_output(data[i].gpio, 0);
        gpio_set_value(data[i].gpio, 0);
        hrtimer_init(&data[i].timer, CLOCK_REALTIME, HRTIMER_MODE_REL);
        data[i].timer.function = pwm_hrtimer_handler;
        hrtimer_start(&data[i].timer, ktime_set(0, 1000), HRTIMER_MODE_REL);
    }

    platform_set_drvdata(pdev, dev);
    return 0;

err3:
    class_destroy(dev->class);
    cdev_del(&dev->cdev);
    unregister_chrdev_region(dev->dev, num);
err2:
    kfree(dev->data);
err1:
err0:
    return -1;
}

static int motor_remove(struct platform_device *pdev)
{
    struct motor_dev *dev = platform_get_drvdata(pdev);
    for (int i = 0; i < dev->num; i++) {
        gpio_free(dev->data[i].gpio);
        hrtimer_cancel(&dev->data[i].timer);
        device_destroy(dev->class, dev->dev + i);
    }

    class_destroy(dev->class);
    cdev_del(&dev->cdev);
    unregister_chrdev_region(dev->dev, dev->num);

    kfree(dev->data);

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