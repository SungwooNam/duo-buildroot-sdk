#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "dialect.h"

#define DEVICE_NAME "dialect"
#define BUF_LEN 80

static int major;
static char msg[BUF_LEN];
static char *msg_ptr;

static int device_open(struct inode *inode, struct file *file) {
    msg_ptr = msg;
    try_module_get(THIS_MODULE);
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    module_put(THIS_MODULE);
    return 0;
}

static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t * offset) {
    int bytes_read = 0;

    if (*msg_ptr == 0)
        return 0;

    while (length && *msg_ptr) {
        put_user(*(msg_ptr++), buffer++);
        length--;
        bytes_read++;
    }

    return bytes_read;
}

static ssize_t device_write(struct file *filp, const char __user *buffer, size_t length, loff_t * offset) {
    int i;
    for (i = 0; i < length && i < BUF_LEN; i++)
        get_user(msg[i], buffer + i);
    msg_ptr = msg;
    return i;
}

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

static int __init dialect_dev_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ALERT "Registering dialect device failed with %d\n", major);
        return major;
    }
    printk(KERN_INFO "I was assigned major number %d. To talk to\n", major);
    printk(KERN_INFO "the driver, create a dev file with\n");
    printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, major);
    printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
    printk(KERN_INFO "the device file.\n");
    printk(KERN_INFO "Remove the device file and module when done.\n");
    return 0;
}

static void __exit dialect_dev_exit(void) {
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "Goodbye, world!\n");
}

module_init(dialect_dev_init);
module_exit(dialect_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sungwooo Nam");
MODULE_DESCRIPTION("A dialect device driver");