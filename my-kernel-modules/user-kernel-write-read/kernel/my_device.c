/*
 * To make the user space access the driver (no sudo) the only way
 * I found was to register the module using misc registration (i.e.
 * register the module as misc device and NOT char device)
 */
#define MISC_REGISTRATION
// #define CHRDEV_REGISTRATION

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#include <linux/device.h> // class_create
#include <linux/ioctl.h> // to define _IOR and _IOW, see Documentation/ioctl/ioctl-number.txt
// #include <linux/cdev.h> // to include if cdev_init is needed
#ifdef MISC_REGISTRATION
#include <linux/miscdevice.h> // to register as misc in __init and get user permission
#endif

/*
 * When using ioctl, it is a good convention to assign a magic number
 * to uniquelly identify the module. Note, the user space app will refer
 * to the same magic number, so that there is an extra check that the
 * user space app is addressing the correct module.
 *
 * See ldd3/ch06.pdf for more details.
 */
#define MY_MAGIC ']'
#define READ_IOCTL _IOR(MY_MAGIC, 0, int)
#define WRITE_IOCTL _IOW(MY_MAGIC, 1, int)

#ifdef CHRDEV_REGISTRATION
static int major;
static struct class *device_class;
static dev_t devn;
static struct device *ptr_error;
#endif

// Used to store the message received via ioctl
static char msg[200];

// Method used to specify what to do during /dev/file read. See fops struct
static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset)
{
  	return simple_read_from_buffer(buffer, length, offset, msg, 200);
}

// Method used to specify what to do during /dev/file write. See fops struct
static ssize_t device_write(struct file *filp, const char __user *buffer, size_t len, loff_t *off)
{
	if (len > 199)
		return -EINVAL;
	raw_copy_from_user(msg, buffer, len);
	msg[len] = '\0';
	return len;
}

// Method used to specify what to do during /dev/file ioctl. See fops struct
long int device_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
	int len = 200;
	char buf[len];
	switch(cmd) {
	case READ_IOCTL:
	  raw_copy_to_user((char *)arg, buf, len);
	  break;
	
	case WRITE_IOCTL:
	  raw_copy_from_user(buf, (char *)arg, len);
	  break;

	default:
	  printk(KERN_INFO "Switch ioctl failed\n");
	  printk(KERN_INFO "READ_IOCTL: %d\n",READ_IOCTL);
	  printk(KERN_INFO "WRITE_IOCTL: %d\n",WRITE_IOCTL);
	  printk(KERN_INFO "cmd: %d\n",cmd);
	  
	  return -ENOTTY;
	}
	return (long int) len;
}

// Struct specifiying what to do when accessing /dev/file
static struct file_operations fops = {
	#ifdef MISC_REGISTRATION
	.owner = THIS_MODULE,
	#endif
	.read = device_read, 
	.write = device_write,
	.unlocked_ioctl = device_ioctl,
};

#ifdef MISC_REGISTRATION
static struct miscdevice my_device_driver = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "my_device",
	.fops = &fops,
	// User permissions:
	// http://stackoverflow.com/questions/10329388/how-to-register-dev-with-specific-mod
	.mode = 0666,
};
#endif

MODULE_LICENSE("Dual BSD/GPL");

static int __init cdevexample_module_init(void)
{

	#ifdef CHRDEV_REGISTRATION
	/* register_chrdev */
	// Using 0 as first parameter, the kernel will dinamically assign a major number
	major = register_chrdev(0, "my_device", &fops);
	if (major < 0) {
     		printk ("Registering the character device failed with %d\n", major);
	     	return major;
	}
	printk("cdev example: assigned major: %d\n", major);
	printk("create node with mknod /dev/my_device c %d 0\n", major);

	// http://stackoverflow.com/questions/11765395/arm-char-device-driver-initialization-isnt-creating-dev-file

	// We fix the minor number to 0, because my driver only drives this module
	devn = MKDEV(major,0);
	// http://stackoverflow.com/questions/12047946/trouble-calling-ioctl-from-user-space-c

	/* class_create */
    	device_class = class_create(THIS_MODULE, "my_device");
    	if (IS_ERR(device_class))
    	{
    		unregister_chrdev(major, "my_device");
    		printk(KERN_INFO "Class creation failed\n");
    		return PTR_ERR(device_class);
    	}

	// Trying to get access from user:
	/*
	// Instead of using udev rules, this line gives the module access from user space
	device_class->dev_uevent = my_dev_uevent;
	*/
	// device_class->class_attrs=uwb_class_attrs;

	/* device_create */
    	ptr_error = device_create(device_class, NULL, devn, NULL, "my_device");
    	if (IS_ERR(ptr_error))
    	{
    		class_destroy(device_class);
    		unregister_chrdev(major, "my_device");
    		printk(KERN_INFO "Device creation failed\n");
    		return PTR_ERR(ptr_error);
    	}

	/*added to test Udev*/
	/*
	int ret;
	cdev_init(&c_dev, &fops);
	if ((ret = cdev_add(&c_dev, major, 1)) < 0)
	{
		device_destroy(device_class, major);
		class_destroy(device_class);
		unregister_chrdev_region(major, 1);
		return ret;
	}
	*/
	#endif

	#ifdef MISC_REGISTRATION
	int ret;

	ret = misc_register(&my_device_driver);
	if (ret < 0){
		printk(KERN_INFO "misc_register failed\n");
		return ret;
	}

	#endif

	return 0;
}

static void __exit cdevexample_module_exit(void)
{
	#ifdef CHRDEV_REGISTRATION
	device_destroy(device_class, devn);
    	class_destroy(device_class);
	unregister_chrdev(major, "my_device");
	#endif

	#ifdef MISC_REGISTRATION
	misc_deregister(&my_device_driver);
	#endif
}  

module_init(cdevexample_module_init);
module_exit(cdevexample_module_exit);
