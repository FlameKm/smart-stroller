#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>


#define DEVICE_NAME "servo"
#define PWM_PERIOD_US 20000// 20ms

#define SERVO_SET_DUTY _IO('S', 0)//uint32_t
#define SERVO_GET_DUTY _IO('S', 1)
#define SERVO_START _IO('S', 2)
#define SERVO_STOP _IO('S', 3)

enum PWM_STATE
{
    PWM_DISABLE = 0,
    PWM_START,
    PWM_RISING,
    PWM_FALLING,
};

struct limit {
    uint32_t min;
    uint32_t max;
};

struct servo_data {
    struct miscdevice miscdev;
    struct gpio_desc *gpio;
    struct hrtimer timer;
    int servo_state;
    uint32_t duty_us;
    uint32_t period_us;
    struct limit duty_limit;
};

static int start_pwm_hrtimer(struct servo_data *data, enum PWM_STATE state, uint32_t delay_us)
{
    ktime_t ktime;
    data->servo_state = state;
    ktime = ktime_set(0, delay_us * 1000);
    hrtimer_start(&data->timer, ktime, HRTIMER_MODE_REL);
    return 0;
}

enum hrtimer_restart servo_hrtimer_handler(struct hrtimer *hrtimer)
{
    struct servo_data *data = container_of(hrtimer, struct servo_data, timer);
    switch (data->servo_state) {
        case PWM_START:
        case PWM_RISING:
            gpiod_set_value(data->gpio, 1);
            start_pwm_hrtimer(data, PWM_FALLING, data->duty_us);
            break;
        case PWM_FALLING:
            gpiod_set_value(data->gpio, 0);
            start_pwm_hrtimer(data, PWM_RISING, data->period_us - data->duty_us);
            break;
        case PWM_DISABLE:
            gpiod_set_value(data->gpio, 0);
            return HRTIMER_NORESTART;
        default:
            break;
    }
    return HRTIMER_NORESTART;
}

static int servo_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static int servo_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static long servo_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct servo_data *data = container_of(filp->private_data, struct servo_data, miscdev);
    switch (cmd) {
        case SERVO_SET_DUTY:
            if (copy_from_user(&data->duty_us, (uint32_t *)arg, sizeof(uint32_t)))
                return -EFAULT;
            break;
        case SERVO_GET_DUTY:
            if (copy_to_user((uint32_t *)arg, &data->duty_us, sizeof(uint32_t)))
                return -EFAULT;
            break;
        case SERVO_START:
            start_pwm_hrtimer(data, PWM_START, 1000);
            break;
        case SERVO_STOP:
            start_pwm_hrtimer(data, PWM_DISABLE, 1000);
            break;
        default: return -EINVAL;
    }
    return 0;
}

static const struct file_operations servo_fops = {
    .owner = THIS_MODULE,
    .open = servo_open,
    .release = servo_release,
    .unlocked_ioctl = servo_ioctl,
};

static int servo_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct servo_data *data;
    int ret;
    int gpio_pin;
    data = devm_kzalloc(dev, sizeof(struct servo_data), GFP_KERNEL);
    if(IS_ERR(data))
        return PTR_ERR(data);
    ret = of_property_read_u32(dev->of_node, "signal-gpio", &gpio_pin);
    if(ret < 0) {
        dev_err(dev, "Failed to get GPIO\n");
        goto err;
    }
    dev_info(dev, "gpio pin: %d\n", gpio_pin);
    data->gpio = gpio_to_desc(gpio_pin);
    if (IS_ERR(data->gpio)) {
        dev_err(dev, "Failed to get GPIO\n");
        return PTR_ERR(data->gpio);
    }
    ret = gpiod_direction_output(data->gpio, 0);
    if (ret) {
        dev_err(dev, "Failed to set GPIO direction\n");
        goto err;
    }
    gpiod_set_value(data->gpio, 0);

    data->period_us = PWM_PERIOD_US;
    data->servo_state = PWM_DISABLE;
    data->duty_limit.min = 500;//us
    data->duty_limit.max = 2500;

    hrtimer_init(&data->timer, CLOCK_REALTIME, HRTIMER_MODE_REL);
    data->timer.function = servo_hrtimer_handler;

    data->miscdev.minor = MISC_DYNAMIC_MINOR;
    data->miscdev.name = DEVICE_NAME;
    data->miscdev.fops = &servo_fops;
    ret = misc_register(&data->miscdev);
    if (ret < 0) {
        dev_err(dev, "Failed to register misc device\n");
        goto err;
    }

    platform_set_drvdata(pdev, data);
    return 0;
err:
    return ret;
}

static int servo_remove(struct platform_device *pdev)
{
    struct servo_data *data = platform_get_drvdata(pdev);

    hrtimer_cancel(&data->timer);
    misc_deregister(&data->miscdev);

    return 0;
}

static const struct of_device_id servo_of_match[] = {
    {
        .compatible = "yc,servo",
    },
    {/* sentinel */}};

static struct platform_driver servo_driver = {
    .driver = {
        .name = "servo",
        .of_match_table = servo_of_match,
    },
    .probe = servo_probe,
    .remove = servo_remove,
};

module_platform_driver(servo_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Servo driver using GPIO and PWM");