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
#define HW508PIN (10)
#define DEVICE_MAJOR 230
#define DEVICE_NAME "hw508"

unsigned char buf[6];
unsigned char check_flag;

void control_led(int value) {
    gpio_direction_output(HW508PIN, value);
}

static ssize_t buzzer_write(struct file *file, const char __user *buffer, size_t size, loff_t *offset) {
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
        control_led(0);  // Turn off buzzer
    } else if (value == '1') {
        control_led(1);  // Turn on buzzer
    } else {
        return -EINVAL;  // Error: unsupported data
    }

    return size;  // Return number of bytes written
}

static int buzzer_open(struct inode *inode, struct file *file) {
    printk("open in kernel\n");
    return 0;
}

static int buzzer_release(struct inode *inode, struct file *file) {
    printk("Led release\n");
    return 0;
}

static struct file_operations buzzer_dev_fops = {
    .owner   = THIS_MODULE,
    .open    = buzzer_open,
    .write   = buzzer_write,
    .release = buzzer_release,
};

static struct class *buzzer_class;
static struct device *buzzer_device;

static int __init buzzer_dev_init(void) {
    int ret;

    ret = register_chrdev(DEVICE_MAJOR, DEVICE_NAME, &buzzer_dev_fops);
    if (ret < 0) {
        printk(KERN_ERR "%s: Registering device %s with major %d failed with %d\n",
               __func__, DEVICE_NAME, DEVICE_MAJOR, ret);
        return ret;
    }
    printk("HW508 driver register success!\n");

    buzzer_class = class_create(THIS_MODULE, "hw508");
    if (IS_ERR(buzzer_class)) {
        printk(KERN_WARNING "Can't create class %s\n", DEVICE_NAME);
        unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
        return PTR_ERR(buzzer_class);
    }

    buzzer_device = device_create(buzzer_class, NULL, MKDEV(DEVICE_MAJOR, 0), NULL, DEVICE_NAME);
    if (IS_ERR(buzzer_device)) {
        printk(KERN_WARNING "Can't create device node %s\n", DEVICE_NAME);
        class_destroy(buzzer_class);
        unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
        return PTR_ERR(buzzer_device);
    }

    if (gpio_request(HW508PIN, DEVICE_NAME) < 0) {
        printk(KERN_ERR "%s: Unable to get GPIO %d\n", DEVICE_NAME, HW508PIN);
        device_destroy(buzzer_class, MKDEV(DEVICE_MAJOR, 0));
        class_destroy(buzzer_class);
        unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
        return -EBUSY;
    }

    if (gpio_direction_output(HW508PIN, 0) < 0) {
        printk(KERN_ERR "%s: Unable to set GPIO %d as output\n", DEVICE_NAME, HW508PIN);
        gpio_free(HW508PIN);
        device_destroy(buzzer_class, MKDEV(DEVICE_MAJOR, 0));
        class_destroy(buzzer_class);
        unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
        return -EBUSY;
    }

    printk(KERN_INFO "%s: GPIO %d successfully configured as output\n", DEVICE_NAME, HW508PIN);
    return 0;
}

static void __exit buzzer_dev_exit(void) {
    gpio_free(HW508PIN);
    device_destroy(buzzer_class, MKDEV(DEVICE_MAJOR, 0));
    class_destroy(buzzer_class);
    unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
}

module_init(buzzer_dev_init);
module_exit(buzzer_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("WWW");
MODULE_DESCRIPTION("White Buzzer Device Driver");
