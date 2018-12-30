A simple kernel module and user space app communicating messages.
The user app writes a message in the module, the module stores it,
and make it available when the user space app reads the module.

HOW IT WORKS:
- build the user space app: cd user, make
- build the kernel module: cd kernel, make
- load the kernel module: sudo insmode my_device.ko
- make sure the device is properly registered, by running ls /dev
- execute the user app (in user space, by default): ./final_user.x
- remove the kernel module: sudo rmmode my_device.ko

KERNEL IMPLEMENTATION:
By default, the module is registered as misc-device, giving the
user space app the possibility to access /dev/my_device from user space.

If the "CHRDEV_REGISTRATION" is uncommented, the driver is registered as
char, and in order to access the device we have to run "sudo ./final_user.x"
