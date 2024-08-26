#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
// #include <mach/gpio.h>
//#include <asm-generic/uaccess.h>
#include <linux/spinlock.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>  //copy_to/from_user()
#include <linux/gpio.h>     //GPIO
#include <linux/err.h>

// LED is connected to this GPIO
#define LEDPIN (17)
#define DEVICE_MAJOR 229
#define DEVICE_NAME "whiteLed"

unsigned char buf[6];
unsigned char check_flag;

void control_led(int value) {
    gpio_direction_output(LEDPIN, value);
}

static ssize_t whiteLed_write(struct file *file, const char __user *buffer, size_t size, loff_t *offset) {
    unsigned char value;
    int ret;

    if (size != 1) {
        return -EINVAL;  // Error: incorrect data size
    }

    ret = get_user(value, buffer);
    if (ret) {
        return ret;  // Return error code
    }

    if (value == '0') {
        control_led(0);  // Turn off LED
    } else if (value == '1') {
        control_led(1);  // Turn on LED
    } else {
        return -EINVAL;  // Error: unsupported data
    }

    return size;  // Return number of bytes written
}

static int whiteLed_open(struct inode *inode, struct file *file) {
    printk("open in kernel\n");
    return 0;
}

static int whiteLed_release(struct inode *inode, struct file *file) {
    printk("Led release\n");
    return 0;
}

static struct file_operations whiteLed_dev_fops = {
    .owner   = THIS_MODULE,
    .open    = whiteLed_open,
    .write   = whiteLed_write,
    .release = whiteLed_release,
};

static struct class *whiteLed_class;
static struct device *whiteLed_device;

static int __init whiteLed_dev_init(void) {
    int ret;

    ret = register_chrdev(DEVICE_MAJOR, DEVICE_NAME, &whiteLed_dev_fops);
    if (ret < 0) {
        printk(KERN_ERR "%s: Registering device %s with major %d failed with %d\n",
               __func__, DEVICE_NAME, DEVICE_MAJOR, ret);
        return ret;
    }
    printk("WHITE LED driver register success!\n");

    whiteLed_class = class_create(THIS_MODULE, "whiteLed");
    if (IS_ERR(whiteLed_class)) {
        printk(KERN_WARNING "Can't create class %s\n", DEVICE_NAME);
        unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
        return PTR_ERR(whiteLed_class);
    }

    whiteLed_device = device_create(whiteLed_class, NULL, MKDEV(DEVICE_MAJOR, 0), NULL, DEVICE_NAME);
    if (IS_ERR(whiteLed_device)) {
        printk(KERN_WARNING "Can't create device node %s\n", DEVICE_NAME);
        class_destroy(whiteLed_class);
        unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
        return PTR_ERR(whiteLed_device);
    }

    if (gpio_request(LEDPIN, DEVICE_NAME) < 0) {
        printk(KERN_ERR "%s: Unable to get GPIO %d\n", DEVICE_NAME, LEDPIN);
        device_destroy(whiteLed_class, MKDEV(DEVICE_MAJOR, 0));
        class_destroy(whiteLed_class);
        unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
        return -EBUSY;
    }

    if (gpio_direction_output(LEDPIN, 0) < 0) {
        printk(KERN_ERR "%s: Unable to set GPIO %d as output\n", DEVICE_NAME, LEDPIN);
        gpio_free(LEDPIN);
        device_destroy(whiteLed_class, MKDEV(DEVICE_MAJOR, 0));
        class_destroy(whiteLed_class);
        unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
        return -EBUSY;
    }

    printk(KERN_INFO "%s: GPIO %d successfully configured as output\n", DEVICE_NAME, LEDPIN);
    return 0;
}

static void __exit whiteLed_dev_exit(void) {
    gpio_free(LEDPIN);
    device_destroy(whiteLed_class, MKDEV(DEVICE_MAJOR, 0));
    class_destroy(whiteLed_class);
    unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
}

module_init(whiteLed_dev_init);
module_exit(whiteLed_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("WWW");
MODULE_DESCRIPTION("White LED Device Driver");
