#include <linux/cdev.h>     /* char device stuff */
#include <linux/delay.h>     /* msleep */
#include <linux/errno.h>    /* error codes */
#include <linux/fs.h> 	    /* file stuff */
#include <linux/init.h>       /* module_init, module_exit */
#include <linux/kernel.h>   /* printk() */
#include <linux/kthread.h>   /* kthread_create */
#include <linux/module.h>     /* version info, MODULE_LICENSE, MODULE_AUTHOR, printk() */
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include "tnvu7.h"

MODULE_DESCRIPTION("Tnvu7 Midterm Q1");
MODULE_LICENSE("GPL");


#define NO_CHANNELS 4
#define BUF_LEN 512

static int device_file_major_number = 0;
static int peripheralChannelIndex;
static PERIPHERAL_INFO peripheralInfo;
static const char device_name[] = "tnvu7";
static char peripheralChannel[NO_CHANNELS][BUF_LEN];

// Function prototypes
int register_device(void);
void unregister_device(void);
static int midterm_tnvu7_init(void);
static void midterm_tnvu7_exit(void);
static int midterm_tnvu7_open(struct inode *inode, struct file *file);
static int midterm_tnvu7_close(struct inode *inode, struct file *file);
static long midterm_tnvu7_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

// File operations structure
static const struct file_operations midterm_tnvu7_fops = {
    .owner = THIS_MODULE,
    .open = midterm_tnvu7_open,
    .release = midterm_tnvu7_close,
    .unlocked_ioctl = midterm_tnvu7_ioctl,
};

static int midterm_tnvu7_init(void)
{
    int result = 0;
    printk( KERN_NOTICE "midterm-tnvu7 has been successfully connected\n" );

    result = register_device();
    return result;
}

/*===============================================================================================*/
static void midterm_tnvu7_exit(void)
{
    printk( KERN_NOTICE "midterm-tnvu7 has been successfully disconnected\n" );
    unregister_device();
}

/*===============================================================================================*/
module_init(hardware_device_init);
module_exit(hardware_device_exit);

int register_device(void)
{
    int result = 0;

    printk( KERN_NOTICE "Peripheral-Writer: register_device() is called.\n" );
    result = register_chrdev( 0, device_name, &simple_driver_fops );
    if( result < 0 )
    {
        printk( KERN_WARNING "Peripheral-Writer: can\'t register character device with errorcode = %i\n", result );
        return result;
    }

    device_file_major_number = result;
    printk( KERN_NOTICE "Peripheral-Writer: registered character device with major number = %i and minor numbers 0...255\n"
        , device_file_major_number );
    peripheralChannelIndex = 3;
    peripheralInfo.num_channels = NO_CHANNELS;
    peripheralInfo.size_channel = BUF_LEN;

    return 0;
}

void unregister_device(void)
{
    printk( KERN_NOTICE "Peripheral-Writer: unregister_device() is called\n" );
    if(device_file_major_number != 0)
    {
        unregister_chrdev(device_file_major_number, device_name);
    }
}

static int midterm_tnvu7_open(struct inode *inode, struct file *file)
{
    printk( KERN_INFO "Peripheral-Writer: open() is called\n");
    return 0;
}

static int midterm_tnvu7_close(struct inode *inode, struct file *file)
{
    printk( KERN_INFO "Peripheral-Writer: close() is called\n");
    return 0;
}


static long midterm_tnvu7_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
        case MIDTERM_GET_INFO:
            copy_to_user((PERIPHERAL_INFO*) arg, &peripheralInfo, sizeof(peripheralInfo));
            printk(KERN_INFO "num_channels = %d size_channel = %d\n", peripheralInfo.num_channels, peripheralInfo.size_channel);
            break;
            
        case MIDTERM_GET_CHANNEL_INDEX:
            copy_to_user((int*) arg, &peripheralChannelIndex, sizeof(peripheralChannelIndex));
            printk(KERN_INFO "peripheralChannelIndex = %d\n", peripheralChannelIndex);
            break;
            
        case MIDTERM_SET_CHANNEL_INDEX:
            copy_from_user(&peripheralChannelIndex, (int*)arg, sizeof(peripheralChannelIndex));
            printk(KERN_INFO "peripheralChannelIndex = %d\n", peripheralChannelIndex);
            break;
            
        default:
            return -EINVAL;
    }
    
    return 0;
}