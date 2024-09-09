#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/completion.h>
#include <linux/of.h>
#include <linux/cdev.h>
#include <linux/slab.h>

#define DRIVER_NAME "army_dma"
#define DRIVER_CLASS "army_dma_class"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("sungwoo");
MODULE_DESCRIPTION("dma test");

#define DMA_BUFFER_SIZE 1024

struct gs_module_dev {
	dev_t devno;
	struct cdev cdev;
	struct class *class;
	struct device *device;
};

static struct gs_module_dev gs_mdev;

struct gs_dev_info
{
	struct gs_module_dev *mdev;
    
    struct dma_chan * dma_chan;
    dma_addr_t dma_src_handle;
    dma_addr_t dma_dst_handle;
    void *dma_src_buffer;
    void *dma_dst_buffer;
    struct completion dma_complete;
};  

static void dma_callback(void *completion)
{
    complete(completion);
}

static int device_open(struct inode *inode, struct file *filp) 
{
	// dev_info is unique for each open allowing mulitple process to open this device
	struct gs_dev_info *dev_info = kzalloc( sizeof(struct gs_dev_info), GFP_KERNEL );

	pr_info( DRIVER_NAME " : opening\n" );
	{
		struct gs_module_dev *mdev;
		mdev = container_of(inode->i_cdev, struct gs_module_dev, cdev);
		dev_info->mdev = mdev;
		filp->private_data = dev_info;
	}

    printk(KERN_INFO "DMA Test: Opening device\n");

    // Request a DMA channel
    dev_info->dma_chan = dma_request_chan(dev_info->mdev->device, "memcpy");
    if (IS_ERR(dev_info->dma_chan)) {
        printk(KERN_ERR "DMA Test: Failed to request DMA channel\n");
        return PTR_ERR(dev_info->dma_chan);
    }

    // Allocate coherent memory for source buffer
    dev_info->dma_src_buffer = dma_alloc_coherent(dev_info->mdev->device, DMA_BUFFER_SIZE, &dev_info->dma_src_handle, GFP_KERNEL);
    if (!dev_info->dma_src_buffer) {
        printk(KERN_ERR "DMA Test: Failed to allocate source buffer\n");
        dma_release_channel(dev_info->dma_chan);
        return -ENOMEM;
    }

    // Allocate coherent memory for destination buffer
    dev_info->dma_dst_buffer = dma_alloc_coherent(dev_info->mdev->device, DMA_BUFFER_SIZE, &dev_info->dma_dst_handle, GFP_KERNEL);
    if (!dev_info->dma_dst_buffer) {
        printk(KERN_ERR "DMA Test: Failed to allocate destination buffer\n");
        dma_free_coherent(dev_info->mdev->device, DMA_BUFFER_SIZE, dev_info->dma_src_buffer, dev_info->dma_src_handle);
        dma_release_channel(dev_info->dma_chan);
        return -ENOMEM;
    }

    return 0;
}

static int device_release(struct inode *inode, struct file *filp) 
{
	struct gs_dev_info *di;

	pr_info( DRIVER_NAME " : closing\n" );
	di = (struct gs_dev_info *)filp->private_data;

    dma_free_coherent(di->mdev->device, DMA_BUFFER_SIZE, di->dma_src_buffer, di->dma_src_handle);
    dma_free_coherent(di->mdev->device, DMA_BUFFER_SIZE, di->dma_dst_buffer, di->dma_dst_handle);

    dma_release_channel(di->dma_chan);

	kfree( di );

    return 0;
}

static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t * offset) 
{
    printk(KERN_INFO "read ... \n");
    return 0;
}

static ssize_t device_write(struct file *filp, const char __user *buffer, size_t length, loff_t * offset) 
{
	struct gs_dev_info *di;
    struct dma_async_tx_descriptor *tx;
    dma_cookie_t cookie;
    enum dma_ctrl_flags flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT;

    printk(KERN_INFO "write ... \n");

	di = (struct gs_dev_info *)filp->private_data;

    // Initialize source buffer with test data
    memset(di->dma_src_buffer, 0xAA, DMA_BUFFER_SIZE);

    // Prepare the DMA transfer
    tx = di->dma_chan->device->device_prep_dma_memcpy(di->dma_chan, di->dma_dst_handle, di->dma_src_handle, DMA_BUFFER_SIZE, flags);
    if (!tx) {
        printk(KERN_ERR "DMA Test: Failed to prepare DMA memcpy\n");
        return -EIO;
    }

    // Set the callback function
    tx->callback = dma_callback;
    tx->callback_param = &di->dma_complete;

    // Submit the DMA transfer
    cookie = tx->tx_submit(tx);
    if (dma_submit_error(cookie)) {
        printk(KERN_ERR "DMA Test: Failed to submit DMA transfer\n");
        return -EIO;
    }

    // Start the DMA transfer
    dma_async_issue_pending(di->dma_chan);

    // Wait for the DMA transfer to complete
    wait_for_completion(&di->dma_complete);

    // Verify the DMA transfer
    if (memcmp(di->dma_src_buffer, di->dma_dst_buffer, DMA_BUFFER_SIZE) == 0) {
        printk(KERN_INFO "DMA Test: Transfer successful\n");
    } else {
        printk(KERN_ERR "DMA Test: Transfer failed\n");
    }

    return length;
}

static struct file_operations fops = 
{
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_release,
	.read = device_read,
	.write = device_write,
};

static int __init ModuleInit(void) 
{
	pr_info( "%s %s : loading ... \n", DRIVER_CLASS, DRIVER_NAME );

	if( alloc_chrdev_region(&gs_mdev.devno, 0, 1, DRIVER_NAME) < 0) 
	{
		pr_err( "%s %s : failed at alloc_chrdev_region \n", DRIVER_CLASS, DRIVER_NAME );
		return -1;
	}
	pr_info( "%s : registed as major: %d, minor: %d\n", DRIVER_NAME, gs_mdev.devno >> 20, gs_mdev.devno && 0xfffff);

	if(( gs_mdev.class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) 
	{
		pr_err( "%s %s : failed at class_create \n", DRIVER_CLASS, DRIVER_NAME );
		goto ClassError;
	}

	if( ( gs_mdev.device = device_create( gs_mdev.class, NULL, gs_mdev.devno, NULL, DRIVER_NAME)) == NULL) 
	{
		pr_err( "%s %s : failed at device_create \n", DRIVER_CLASS, DRIVER_NAME );
		goto FileError;
	}

	cdev_init(&gs_mdev.cdev, &fops);

	if(cdev_add(&gs_mdev.cdev, gs_mdev.devno, 1) == -1) 
	{
		pr_err( "%s %s : failed at cdev_add \n", DRIVER_CLASS, DRIVER_NAME );
		goto AddError;
	}

	return 0;
AddError:
	device_destroy( gs_mdev.class, gs_mdev.devno);
FileError:
	class_destroy( gs_mdev.class );
ClassError:
	unregister_chrdev_region( gs_mdev.devno, 1);
	return -1;	
}

static void __exit ModuleExit(void) 
{
	pr_info( "%s %s : unloading ... \n", DRIVER_CLASS, DRIVER_NAME );

	cdev_del(&gs_mdev.cdev);
	device_destroy( gs_mdev.class, gs_mdev.devno);
	class_destroy( gs_mdev.class );
	unregister_chrdev_region( gs_mdev.devno, 1);
}

module_init(ModuleInit);
module_exit(ModuleExit);
