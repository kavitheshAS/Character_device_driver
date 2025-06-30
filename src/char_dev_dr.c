#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h> //has a lot these data structures and functions, like the ssize_t etc 
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h> //this is needed for kmalloc()
#include <linux/uaccess.h> //to copy to and from user

#define mem_size 1024

dev_t dev=0; //represents the device numbers(major and minor)
static struct class *dev_class; //represents device classes for udev
//each device in /dev is represented internally by a struct Pointed by *dev_class?
//udev is a device manager for the linux kernel, it primarily manages device nodes in /dev
static struct cdev chr_cdev; //represents the character device structure
uint8_t *kernel_buffer; //represents the kernel memory kernel_buffer


// you need open, close(release), read, write function for a basic device driver
// and the others like init and exit for initializing and remving the module
static int char_open(struct inode *inode, struct file *file);
static int char_release(struct inode *inode, struct file *file);
static ssize_t char_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
//char __user *buf is a pointer to a buffer ni userspace memory where data should be written
//__user is a kernel macro that marks the pointer as coming from userspace, this helps with static analysis and safety
//accessing the pointer directly is a bad practice, use helper fns like copy_to_user() etc
static ssize_t char_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);
//size_t is unsigned and ssize_t is signed, more here https://manpages.ubuntu.com/manpages/lunar/man3/size_t.3type.html
static int __init char_driver_init(void);
static void __exit char_driver_exit(void);

//file operations structure
static struct file_operations fops={
  .owner=THIS_MODULE,
  .read=char_read,
  .write=char_write,
  .open=char_open,
  .release=char_release,
};

static int char_open(struct inode *inode, struct file *file){
  if((kernel_buffer=kmalloc(mem_size,GFP_KERNEL))==0){
    printk(KERN_INFO "cannot allocate memory!\n");
    return -1;
  }
  //while kmalloc() can return NULL if it fails, and using NULL can be bad, hence the check
  printk(KERN_INFO "device file opened!\n");
  return 0;
}

static int char_release(struct inode *inode, struct file *file){
  kfree(kernel_buffer);
  //we dont have a check here because, by structure of kfree, it returns nothing if kfree fails
  printk(KERN_INFO "device file is closed1!\n");
  return 0;
}

static ssize_t char_read(struct file *filp, char __user *buf, size_t len, loff_t *off){
  //copy date from kernel space to user space
  if(copy_to_user(buf, kernel_buffer, mem_size)){
    printk(KERN_ERR "failed to copy data to user space\n");
    return -EFAULT;
  }
  printk(KERN_INFO "data read done!!\n");
  return mem_size;
}

static ssize_t char_write(struct file *filp, const char __user *buf, size_t len, loff_t *off){
  //copy data from user space to kernel space
  if(copy_from_user(kernel_buffer,buf, len)){
    printk(KERN_ERR "failed to copy data to user space\n");
    return -EFAULT;
  }
  printk(KERN_INFO "write function\n");
  return len;
}

//the initialization function initializes and registers the character device
//including major number allocation, cdev initialization, class creation and device creation

static int __init char_driver_init(void){
  //alloc major number
  if((alloc_chrdev_region(&dev, 0, 1, "chr_dev"))<0){
    printk(KERN_INFO "cannot allocate major number\n");
    return -1;
  }

  //init cdev structure
  cdev_init(&chr_cdev,&fops);

  //addign character dev to system
  if((cdev_add(&chr_cdev,dev,1))<0){
    printk(KERN_INFO "cannot add the device to the system\n");
    goto r_class;
  }

  //creating struct class
  if((dev_class = class_create("chr_class")) == NULL) {
        printk(KERN_INFO "Cannot create the struct class\n");
        goto r_class;
  }

// Creating device
  if((device_create(dev_class, NULL, dev, NULL, "chr_device")) == NULL) {
        printk(KERN_INFO "Cannot create the Device\n");
        goto r_device;
  }

  printk(KERN_INFO "device driver insert done!!\n");
  return 0;
  
r_device:
  class_destroy(dev_class);

r_class:
  unregister_chrdev_region(dev,1);
  cdev_del(&chr_cdev);
  return -1;
}

void __exit char_driver_exit(void){
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&chr_cdev);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "Device Driver Remove…Done!!!\n");
}

module_init(char_driver_init);
module_exit(char_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("kavithesh");
MODULE_DESCRIPTION("linux character device driver");


