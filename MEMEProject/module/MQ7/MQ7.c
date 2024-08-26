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



//#define PIN (21)

#define DB0 (0)
#define DB1 (1)
#define DB2 (2)
#define DB3 (3)
#define DB4 (4)
#define DB5 (5)
#define DB6 (6)
#define DB7 (14)



#define WR (15)
#define INTR (18)



#define DEVICE_MAJOR 232
#define DEVICE_NAME "MQ7"

typedef unsigned char U8;
unsigned char buf[2];

int data_in(void)
{
	buf[0] = 0;
	buf[1] = 0;
//	buf[0] = gpio_get_value(PIN);
	return 0;
}

int gpio_in(int PI)
{
	return gpio_get_value(PI);
}

void gpio_out(int PI ,int value)   //set gpio is output
{
    gpio_direction_output(PI,value);
}


void read_data(void)
{
	gpio_out(WR,0);
		udelay(200);
	
	while(!gpio_in(INTR));
	
	buf[1] |= gpio_in(DB0) << 0;
	buf[1] |= gpio_in(DB1) << 1;
	buf[1] |= gpio_in(DB2) << 2;
	buf[1] |= gpio_in(DB3) << 3;
	buf[1] |= gpio_in(DB4) << 4;
	buf[1] |= gpio_in(DB5) << 5;
	buf[1] |= gpio_in(DB6) << 6;
	buf[1] |= gpio_in(DB7) << 7;
	gpio_out(WR,1);

}


static int test_open(struct inode *inode, struct file *file)
{
	printk("open in kernel\n");
	return 0;
}


static ssize_t test_read(struct file *file, char* buffer,size_t size ,loff_t *off)
{
		int ret;
        local_irq_disable();
		data_in();
		read_data();
        local_irq_enable();
        
		ret = copy_to_user(buffer,buf,sizeof(buf));
		if(ret < 0)
			{
				printk("copy to user err\n");
				return -EAGAIN;
            }
            else
            {
            	return 0;
			}
         
}

static int test_release(struct inode *inode, struct file *file)
{
	printk("test release\n");
	return 0;
}

static struct file_operations MQ7_dev_fops={
	owner	:	THIS_MODULE,
	open	:	test_open,
	read	:	test_read,
	release	:	test_release,
};
static struct class *MQ7_class;

static int __init test_init(void)
{
	int ret;

	ret =register_chrdev(DEVICE_MAJOR, DEVICE_NAME, &MQ7_dev_fops);
	if (ret < 0) {
		printk(KERN_INFO "%s: registering device %s with major %d failed with %d\n",
		__func__, DEVICE_NAME, DEVICE_MAJOR, DEVICE_MAJOR );
		return(ret);
		}
	printk("MQ7 driver register success!\n");

	MQ7_class = class_create(THIS_MODULE, "MQ7");
		if(IS_ERR(MQ7_class))
		{
			printk(KERN_WARNING "CANT MAKE NODE %d\n",DEVICE_MAJOR);
				return PTR_ERR(MQ7_class);
		}

		device_create(MQ7_class, NULL, MKDEV(DEVICE_MAJOR, 0), NULL, DEVICE_NAME);

		printk("MQ7 driver make node success!\n");


	/*if( gpio_request( PIN, DEVICE_NAME ) < 0 )	// request pin 2
	{
		printk( KERN_INFO "%s: %s unable to get TRIG gpio\n", DEVICE_NAME, __func__ );
		ret = -EBUSY;
		return(ret);
	}*/
		if( gpio_request( DB0, DEVICE_NAME ) < 0 )	// request pin 2
	{
		printk( KERN_INFO "%s: %s unable to get TRIG gpio0\n", DEVICE_NAME, __func__ );
		ret = -EBUSY;
		return(ret);
	}
		if( gpio_request( DB1, DEVICE_NAME ) < 0 )	// request pin 2
	{
		printk( KERN_INFO "%s: %s unable to get TRIG gpio1\n", DEVICE_NAME, __func__ );
		ret = -EBUSY;
		return(ret);
	}
		if( gpio_request( DB2, DEVICE_NAME ) < 0 )	// request pin 2
	{
		printk( KERN_INFO "%s: %s unable to get TRIG gpio2\n", DEVICE_NAME, __func__ );
		ret = -EBUSY;
		return(ret);
	}
		if( gpio_request( DB3, DEVICE_NAME ) < 0 )	// request pin 2
	{
		printk( KERN_INFO "%s: %s unable to get TRIG gpio3\n", DEVICE_NAME, __func__ );
		ret = -EBUSY;
		return(ret);
	}
		if( gpio_request( DB4, DEVICE_NAME ) < 0 )	// request pin 2
	{
		printk( KERN_INFO "%s: %s unable to get TRIG gpio4\n", DEVICE_NAME, __func__ );
		ret = -EBUSY;
		return(ret);
	}
		if( gpio_request( DB5, DEVICE_NAME ) < 0 )	// request pin 2
	{
		printk( KERN_INFO "%s: %s unable to get TRIG gpio5\n", DEVICE_NAME, __func__ );
		ret = -EBUSY;
		return(ret);
	}
		if( gpio_request( DB6, DEVICE_NAME ) < 0 )	// request pin 2
	{
		printk( KERN_INFO "%s: %s unable to get TRIG gpio6\n", DEVICE_NAME, __func__ );
		ret = -EBUSY;
		return(ret);
	}
		if( gpio_request( DB7, DEVICE_NAME ) < 0 )	// request pin 2
	{
		printk( KERN_INFO "%s: %s unable to get TRIG gpio7\n", DEVICE_NAME, __func__ );
		ret = -EBUSY;
		return(ret);
	}
	
	
		if( gpio_request( WR, DEVICE_NAME ) < 0 )	// request pin 2
	{
		printk( KERN_INFO "%s: %s unable to get TRIG gpio10\n", DEVICE_NAME, __func__ );
		ret = -EBUSY;
		return(ret);
	}
	
		if( gpio_request( INTR, DEVICE_NAME ) < 0 )	// request pin 2
	{
		printk( KERN_INFO "%s: %s unable to get TRIG gpio11\n", DEVICE_NAME, __func__ );
		ret = -EBUSY;
		return(ret);
	}
	

	gpio_out(WR,1);
	//gpio_direction_input(PIN);
	gpio_direction_input(DB0);
	gpio_direction_input(DB1);
	gpio_direction_input(DB2);
	gpio_direction_input(DB3);
	gpio_direction_input(DB4);
	gpio_direction_input(DB5);
	gpio_direction_input(DB6);
	gpio_direction_input(DB7);
	gpio_direction_input(INTR);

	return 0 ;
}

static void __exit test_exit(void)
{
	//gpio_free(PIN);
	gpio_free(DB0);
	gpio_free(DB1);
	gpio_free(DB2);
	gpio_free(DB3);
	gpio_free(DB4);
	gpio_free(DB5);
	gpio_free(DB6);
	gpio_free(DB7);
	



	gpio_free(WR);

	gpio_free(INTR);

	device_destroy(MQ7_class,DEVICE_MAJOR);
	class_destroy(MQ7_class);
	unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
}
module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sr");
