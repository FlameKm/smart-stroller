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

#define MOTOR_SET_SPEED 0  // uint32_t
#define MOTOR_GET_SPEED 1
#define MOTOR_START 2 
#define MOTOR_STOP 3

struct motor_data {
    uint32_t pwm_ch;
    uint32_t duty_us;
    uint32_t period_us;
    struct pwm_device *pwm;

    struct device *device;
};

struct motor_dev {
    struct motor_data *data;
    int num;

    dev_t dev;
    struct cdev cdev;
    struct class *class;
};

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
            pwm_config(data->pwm, data->duty_us * 1000, data->period_us * 1000);
            break;
        }
        case MOTOR_GET_SPEED: {
            ret = (int)copy_to_user((void*)arg, &data->duty_us, sizeof(data->duty_us));
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

static int motor_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct device_node *node = pdev->dev.of_node;
    struct motor_dev *dev = NULL;
    struct motor_data *data;
    int num = 0;

    dev = devm_kmalloc(&pdev->dev, sizeof(struct motor_data), GFP_KERNEL);
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
        dev_info(&pdev->dev, "%d %d %d\n", data[i].pwm_ch, data[i].duty_us, data[i].period_us);
        // char pwm_name[6];
        // snprintf(pwm_name, 6, "motor%d", i);
        data[i].pwm = pwm_request(data[i].pwm_ch, "motor");
        if (IS_ERR(data[i].pwm)) {
            dev_err(&pdev->dev, "Failed to request pwm %d\n", i);
            goto err3;
        }
        pwm_config(data[i].pwm, data[i].duty_us * 1000, data[i].period_us * 1000);
        pwm_enable(data[i].pwm);
    }

    platform_set_drvdata(pdev, dev);
    return 0;

    for (int i = 0; i < num; i++) {
        pwm_disable(dev->data[i].pwm);
        pwm_free(dev->data[i].pwm);
    }
err3:
    class_destroy(dev->class);
    cdev_del(&dev->cdev);
    unregister_chrdev_region(dev->dev, num);
err2:
    kfree(dev->data);
err1:
    devm_kfree(&pdev->dev, dev);
err0:
    return -1;
}

static int motor_remove(struct platform_device *pdev)
{
    struct motor_dev *dev = platform_get_drvdata(pdev);
    for (int i = 0; i < dev->num; i++) {
        device_destroy(dev->class, dev->dev + i);
        pwm_disable(dev->data[i].pwm);
        pwm_free(dev->data[i].pwm);
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