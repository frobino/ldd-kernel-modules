Trying to adapt to the interrupt example shown in NoC_sowftware (de1-soc project).

The goal here is to have a TIMER, instead of an INTERRUPT in the kernel module.
The TIMER should "unblock" a user space app waining for the read by using a wait queue,
in the same way the interrupt was comminicating to user space that the interrupt has arrived. 