#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "led_driver"
#define LED_SYSFS_FILE "/sys/class/leds/led0/brightness"

static ssize_t led_driver_read(struct file *file, char *buffer, size_t length, loff_t *offset)
{
    char led_status;
    int error_count = 0;
    ssize_t bytes_read = 0;
    int fd;

    fd = open(LED_SYSFS_FILE, O_RDONLY);
    if (fd < 0) {
        return -EFAULT;
    }

    error_count = read(fd, &led_status, sizeof(led_status));
    if (error_count < 0) {
        close(fd);
        return -EFAULT;
    }

    close(fd);

    bytes_read = copy_to_user(buffer, &led_status, sizeof(led_status));
    if (bytes_read != 0) {
        return -EFAULT;
    }

    return 0;
}

static ssize_t led_driver_write(struct file *file, const char *buffer, size_t length, loff_t *offset)
{
    char command;
    int ret;
    int fd;

    fd = open(LED_SYSFS_FILE, O_WRONLY);
    if (fd < 0) {
        return -EFAULT;
    }

    ret = copy_from_user(&command, buffer, sizeof(command));
    if (ret != 0) {
        close(fd);
        return -EFAULT;
    }

    ret = write(fd, &command, sizeof(command));
    if (ret < 0) {
        close(fd);
        return -EFAULT;
    }

    close(fd);

    return length;
}

static struct file_operations led_driver_fops = {
    .read = led_driver_read,
    .write = led_driver_write,
};

static int __init led_driver_init(void)
{
    int ret;

    ret = register_chrdev(0, DEVICE_NAME, &led_driver_fops);
    if (ret < 0) {
        return ret;
    }

    return 0;
}

static void __exit led_driver_exit(void)
{
    unregister_chrdev(0, DEVICE_NAME);
}

module_init(led_driver_init);
module_exit(led_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ivan Birkovskyi");
MODULE_DESCRIPTION("LED Driver with Character Device Interface");
