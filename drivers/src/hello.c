#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

static struct miscdevice hello_miscdev;


static int hello_open(struct inode *node, struct file *file)
{
    printk(KERN_INFO "Hello, open!\n");
    return 0;
}

static int hello_release(struct inode *node, struct file *file)
{
    printk(KERN_INFO "Hello, release!\n");
    return 0;
}

static struct file_operations hello_fops = {
    .owner = THIS_MODULE,
    .open = hello_open,
    .release = hello_release,
};

static int __init hello_init(void)
{
    printk(KERN_INFO "Hello, driver!\n");

    hello_miscdev.minor = MISC_DYNAMIC_MINOR;
    hello_miscdev.name = "hello";
    hello_miscdev.fops = &hello_fops;
    misc_register(&hello_miscdev);

    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_INFO "Goodbye, driver!\n");
    misc_deregister(&hello_miscdev);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple hello driver");