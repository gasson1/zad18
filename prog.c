
#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>   /* Needed for KERN_INFO */
#include <linux/fs.h>       /* Needed for file operations */
#include <linux/cdev.h>     /* Needed for character device */
#include <linux/uaccess.h>  /* Needed for copy_to_user and copy_from_user */
#include <linux/io.h>
#include <linux/delay.h>

#define GPIO_ADDR_BASE              0x4804C000
#define ADDR_SIZE                   0X1000
#define GPIO_SETDATAOUT_OFFSET      0X194
#define GPIO_CLEARDATAOUT_OFFSET    0X190
#define GPIO_OE_OFFSET              0X134
#define GPIO_DATAIN_OFFSET          0X138

#define LED_VALUE_OFF               (1 << 24)
#define LED_VALUE_ON                ~(1 << 24)

void __iomem *base_addr;
static dev_t dev_num;
static struct cdev my_cdev;

static int hello_open(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t hello_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    char data;
    if (copy_from_user(&data, buf, 1))
        return -EFAULT;

    if (data == 'I' || data == 'B') {
        // Turn LED on
        writel(LED_VALUE_ON, base_addr + GPIO_CLEARDATAOUT_OFFSET);
    } else {
        // Turn LED off
        writel(LED_VALUE_OFF, base_addr + GPIO_SETDATAOUT_OFFSET);
    }

    return 1;
}

static const struct file_operations hello_fops = {
    .owner = THIS_MODULE,
    .open = hello_open,
    .read = hello_read,
};

int init_module(void)
{
    printk(KERN_EMERG "Hello\n");

    base_addr = ioremap(GPIO_ADDR_BASE, ADDR_SIZE);

    // Configure GPIO Pin for LED
    writel(LED_VALUE_OFF, base_addr + GPIO_OE_OFFSET); // Set pin as output
    writel(LED_VALUE_ON, base_addr + GPIO_SETDATAOUT_OFFSET); // Turn LED off initially

    // Initialize character device
    if (alloc_chrdev_region(&dev_num, 0, 1, "hello_led") < 0) {
        printk(KERN_ALERT "Failed to allocate character device region\n");
        return -1;
    }

    cdev_init(&my_cdev, &hello_fops);
    if (cdev_add(&my_cdev, dev_num, 1) < 0) {
        printk(KERN_ALERT "Failed to add character device\n");
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }

    return 0;
}

void cleanup_module(void)
{
    printk(KERN_EMERG "Goodbye\n");

    // Turn off LED
    writel(LED_VALUE_OFF, base_addr + GPIO_CLEARDATAOUT_OFFSET);

    // Cleanup character device
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ivan Birkovskyi");
MODULE_DESCRIPTION("LED driver with character device interface");
