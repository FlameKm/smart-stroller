#include <linux/mm.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/capability.h>
#include <linux/ptrace.h>
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

#define DEVICE_NAME "yc_motor"

struct motor_dev_struct {
    int pwm;
    int speed;
};

static int major = 0;
static struct class *motor_class;
struct motor_dev_struct *motor_dev;

static int motor_open(struct inode *node, struct file *filp)
{
    printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    return 0;
}
static ssize_t motor_read(struct file *filp, char __user *buf, size_t size, loff_t *offset)
{
    printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    return size;
}

static ssize_t motor_write(struct file *filp, const char __user *buf, size_t size, loff_t *offset)
{
    printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    return size;
}

static int motor_release(struct inode *node, struct file *filp)
{
    printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    return 0;
}

static const struct file_operations motor_drv = {
    .owner = THIS_MODULE,
    .read = motor_read,
    .write = motor_write,
    .open = motor_open,
    .release = motor_release,
};

static int motor_probe(struct platform_device *pdev)
{

    // todo get pwms
    // todo multi device

    major = register_chrdev(0, DEVICE_NAME, &motor_drv); /* /dev/led */

    motor_class = class_create(THIS_MODULE, "motor_class");
    if (IS_ERR(motor_class)) {
        return PTR_ERR(motor_class);
    }

    device_create(motor_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME "%d", 0); /* /dev/100ask_led0 */

    return 0;
}

static int motor_remove(struct platform_device *pdev)
{
    device_destroy(motor_class, MKDEV(major, 0));
    class_destroy(motor_class);
    unregister_chrdev(major, DEVICE_NAME);

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