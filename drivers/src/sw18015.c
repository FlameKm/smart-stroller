#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/container_of.h>
#include <linux/timer.h>

#define DEVICE_NAME "sw18015"

#define SW18015_SHAKE_COUNT _IOR('k', 0, int)
#define SW18015_CLEAN _IOR('k', 1, int)

struct sw18015_data {
    struct miscdevice miscdev;
    struct gpio_desc *gpio;
    wait_queue_head_t wait_queue;
    struct timer_list timer;
    int event_flag;
};

static void sw18015_timer_handler(struct timer_list *t)
{
    struct sw18015_data *data = container_of(t, struct sw18015_data, timer);
    wake_up_interruptible(&data->wait_queue);
}

static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    struct sw18015_data *data = (struct sw18015_data *)dev_id;
    mod_timer(&data->timer, jiffies + msecs_to_jiffies(100));
    data->event_flag++;
    return IRQ_HANDLED;
}

static unsigned int sw18015_poll(struct file *filp, poll_table *wait)
{
    struct sw18015_data *data = container_of(filp->private_data, struct sw18015_data, miscdev);
    unsigned int mask = 0;
    poll_wait(filp, &data->wait_queue, wait);

    if (data->event_flag) {
        data->event_flag = false;
        mask |= POLLIN | POLLRDNORM;
    }

    return mask;
}

static long sw18015_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct sw18015_data *data = container_of(file->private_data, struct sw18015_data, miscdev);
    switch (cmd) {
        case SW18015_SHAKE_COUNT: {
            long value = copy_to_user((int *)arg, &data->event_flag, sizeof(int));
            if(value)
                return -EFAULT;
            break;
        }
        case SW18015_CLEAN:
            data->event_flag = 0;
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = sw18015_ioctl,
    .poll = sw18015_poll,
};


static int sw18015_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct sw18015_data *data;
    int ret;
    int gpio_pin;
    data = devm_kzalloc(dev, sizeof(struct sw18015_data), GFP_KERNEL);
    if (!data)
        return -ENOMEM;
    ret = of_property_read_u32(dev->of_node, "AO-gpio", &gpio_pin);
    dev_info(dev, "gpio pin: %d\n", gpio_pin);
    data->gpio = gpio_to_desc(gpio_pin);
    if (IS_ERR(data->gpio)) {
        dev_err(dev, "Failed to get GPIO\n");
        return PTR_ERR(data->gpio);
    }
    ret = gpiod_to_irq(data->gpio);
    if (ret < 0) {
        dev_err(dev, "Failed to get IRQ\n");
        return ret;
    }

    ret = devm_request_irq(dev, ret, gpio_irq_handler, IRQF_TRIGGER_RISING, dev_name(dev), data);
    if (ret < 0) {
        dev_err(dev, "Failed to request IRQ\n");
        return ret;
    }
    timer_setup(&data->timer, sw18015_timer_handler, 0);
    add_timer(&data->timer);
    init_waitqueue_head(&data->wait_queue);
    data->event_flag = 0;

    data->miscdev.minor = MISC_DYNAMIC_MINOR;
    data->miscdev.name = DEVICE_NAME;
    data->miscdev.fops = &fops;

    ret = misc_register(&data->miscdev);
    if (ret < 0) {
        dev_err(dev, "Failed to register misc device\n");
        return ret;
    }

    platform_set_drvdata(pdev, data);

    return 0;
}

static int sw18015_remove(struct platform_device *pdev)
{
    struct sw18015_data *data = platform_get_drvdata(pdev);
    del_timer(&data->timer);
    misc_deregister(&data->miscdev);

    return 0;
}

static const struct of_device_id sw18015_of_match[] = {
    {
        .compatible = "yc,sw18015",
    },
    {/* sentinel */}};

static struct platform_driver sw18015_driver = {
    .driver = {
        .name = "sw18015",
        .of_match_table = sw18015_of_match,
    },
    .probe = sw18015_probe,
    .remove = sw18015_remove,
};

module_platform_driver(sw18015_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("GPIO driver with external trigger");