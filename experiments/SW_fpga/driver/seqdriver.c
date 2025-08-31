/*** Author  :   William Simon <william.simon@epfl.ch>
 ***             Miguel Peon <miguel.peon@epfl.ch>
 *** Date    :   March 2021
 ***/

#include <linux/init.h>          /* needed for module_init and exit */
#include <linux/module.h>
#include <linux/moduleparam.h>   /* needed for module_param */
#include <linux/kernel.h>        /* needed for printk */
#include <linux/types.h>         /* needed for dev_t type */
#include <linux/kdev_t.h>        /* needed for macros MAJOR, MINOR, MKDEV... */
#include <linux/fs.h>            /* needed for register_chrdev_region, file_operations */
#include <linux/interrupt.h>
#include <linux/cdev.h>          /* cdev definition */
#include <linux/slab.h>		       /* kmalloc(),kfree() */
#include <asm/uaccess.h>         /* copy_to copy_from _user */
#include <linux/uaccess.h>
#include <linux/io.h>

/**
 * PL->PS IRQ0 [0-7] : 121 - 128
 * PL->PS IRQ0 [0-7] : 136 - 143
 * Look into: https://docs.amd.com/r/en-US/pg201-zynq-ultrascale-plus-processing-system/Programmable-Logic-Clocks-and-Interrupts
 */
#define DRIVER_NAME "seqdriver"
#define SEQ_IRQ 56  // Hard-coded value of IRQ vector (GIC: 121).

// Structure that mimics the layout of the peripheral registers.
// Vitis HLS skips some addresses in the register file. We introduce
// padding fields to create the right mapping to registers with our structure,
struct TRegs {
  uint32_t control; // 0x00
  uint32_t gier, ier, isr; // 0x04, 0x08, 0x0C
  uint32_t bit_set_ref_1, bit_set_ref_2; // 0x10, 0x14
  uint32_t padding1; // 0x18
  uint32_t nseqt; // 0x1C
  uint32_t padding2; // 0x20
  uint32_t length_ref_1, length_ref_2; //0x24, 0x28
  uint32_t padding3; // 0x2C
  uint32_t bit_set_pat_1, bit_set_pat_2; // 0x30, 0x34
  uint32_t padding4; // 0x38
  uint32_t nseqp; // 0x3C
  uint32_t padding5; // 0x40
  uint32_t length_pat_1, length_pat_2; // 0x44, 0x48
  uint32_t padding6; // 0x4C
  uint32_t output_1, output_2; // 0x50, 0x54
};

// Type of waiting to the accelerator
typedef enum {
  INTERRUPT = 0x0,  // Wait for the interrupt
  POLLING   = 0x1,  // Polling on the status register
  CONTINUE  = 0x2,  // Do not wait and return to the user
} read_type_t;

// Structure used to pass commands between user-space and kernel-space.
struct write_message {
  uint64_t seq_t;         // Pointer to the target sequences
  uint64_t seq_q;         // Pointer to the query sequences
  uint32_t n_seq_t;       // Number of targets
  uint32_t n_seq_q;       // Number of queries
  uint64_t length_seq_t;  // Pointer to the target sequence lengths
  uint64_t length_seq_q;  // Pointer to the target sequence lengths
  uint64_t min_pos;       // Pointer to the min pos array
  read_type_t wait_type;  // Type of waiting to the accelerator
};

int seq_major = 0;
int seq_minor = 0;
module_param(seq_major,int,S_IRUGO);
module_param(seq_minor,int,S_IRUGO);

// We declare a wait queue that will allow us to wait on a condition.
wait_queue_head_t wq;
int flag = 0;

// This structure contains the device information.
struct seq_info {
  int irq;
  uint64_t memStart;
  uint64_t memEnd;
  void __iomem  *baseAddr;
  struct cdev   cdev;            /* Char device structure               */
};

static struct seq_info seq_mem = {SEQ_IRQ, 0x00A0000000, 0x00A000FFFF};
static read_type_t wait_type = INTERRUPT;

// Declare here the user-accessible functions that the driver implements.
int seq_open(struct inode *inode, struct file *filp);
int seq_release(struct inode *inode, struct file *filed_mem);
ssize_t seq_read(struct file *filed_mem, char __user *buf, size_t count, loff_t *f_pos);
ssize_t seq_write(struct file *filed_mem, const char __user *buf, size_t count, loff_t *f_pos);

// IRQ handler function.
static irq_handler_t  seqIRQHandler(unsigned int irq, void *dev_id, struct pt_regs *regs);

// This structure declares the operations that our driver exports for the users.
struct file_operations seq_fops = {
  .owner =    THIS_MODULE,
  .read =     seq_read,
  .write =    seq_write,
  .open =     seq_open,
  .release =  seq_release,
};

// Function that implements system call open() for our driver.
// Initialize the device and enable the interrups here.
int seq_open(struct inode *inode, struct file *filp)
{
  pr_info("SEQ_DRIVER: Performing 'open' operation\n");
  return 0;         
}

// Function that implements system call release() for our driver.
// Used with close() or when the OS closes the descriptors held by
// the process when it is closed (e.g., Ctrl-C).
// Stop the interrupts and disable the device.
int seq_release(struct inode *inode, struct file *filed_mem)
{
  pr_info("SEQ_DRIVER: Performing 'release' operation\n");
  return 0;
}

// The cleanup function is used to handle initialization failures as well.
// Thefore, it must be careful to work correctly even if some of the items
// have not been initialized

void seq_cleanup_module(void)
{
  dev_t devno = MKDEV(seq_major, seq_minor);
  disable_irq(seq_mem.irq);
  free_irq(seq_mem.irq,&seq_mem);
  iounmap(seq_mem.baseAddr);
  release_mem_region(seq_mem.memStart, seq_mem.memEnd - seq_mem.memStart + 1);
  cdev_del(&seq_mem.cdev);
  unregister_chrdev_region(devno, 1);        /* unregistering device */
  pr_info("SEQ_DRIVER: Cdev deleted, seq device unmapped, chdev unregistered\n");
}

// Function that implements system call read() for our driver.
ssize_t seq_read(struct file *filed_mem, char __user *buf, size_t count, loff_t *f_pos)
{
  volatile struct TRegs * slave_regs = (struct TRegs*)seq_mem.baseAddr;
  uint32_t status;

  // Enable interrupts (global and spacific to done).
  iowrite32(1, (volatile void*)(&slave_regs->gier));
  iowrite32(1, (volatile void*)(&slave_regs->ier));
  mb();
    
  flag = 0;
  // Start peripheral (start bit = 1)
  status = ioread32((volatile void*)(&slave_regs->control));
  status |= 1; 
  iowrite32(status, (volatile void*)(&slave_regs->control));
  mb();

  if (wait_type == INTERRUPT) { // INTERRUPT
    while(wait_event_interruptible(wq, flag !=0)) {
      ;
    }
  } else if (wait_type == POLLING) { // POLLING
    do {
      status = slave_regs->control;
    } while ( ( (status & 2) != 2) ); // wait until ap_done==1
  }

  // Disable interrupts.
  iowrite32(0, (volatile void*)&slave_regs->gier);
  iowrite32(0, (volatile void*)&slave_regs->ier);
  mb();

  return 0;
}

// Function that implements system call write() for our driver.
// Returns 1 uint32_t with the number of times the interrupt has been detected.
ssize_t seq_write(struct file *filed_mem, const char __user *buf, size_t count, loff_t *f_pos)
{
  volatile struct TRegs * slave_regs = (struct TRegs*)seq_mem.baseAddr;
  struct write_message message;

  if (count < sizeof(struct write_message)) {
    pr_err("SEQ_DRIVER: USer buffer too small.\n");
    return -1;
  }

  // Copy the information from user-space to the kernel-space buffer.
  if(raw_copy_from_user(&message, buf, sizeof(struct write_message)))
  {
    pr_err("SEQ_DRIVER: Raw copy from user buffer failed.\n");
    return -1;
  }

  // Program the peripheral registers. 
  // Reference / target
  iowrite32((uint32_t)(message.seq_t & 0xFFFFFFFF)        , (volatile void*)(&slave_regs ->  bit_set_ref_1 ));
  iowrite32((uint32_t)(message.seq_t >>32)                , (volatile void*)(&slave_regs ->  bit_set_ref_2 ));
  iowrite32(message.n_seq_t                               , (volatile void*)(&slave_regs ->  nseqt         ));
  iowrite32((uint32_t)(message.length_seq_t & 0xFFFFFFFF) , (volatile void*)(&slave_regs ->  length_ref_1  ));
  iowrite32((uint32_t)(message.length_seq_t >>32)         , (volatile void*)(&slave_regs ->  length_ref_2  ));
  // Query / pattern
  iowrite32((uint32_t)(message.seq_q & 0xFFFFFFFF)        , (volatile void*)(&slave_regs ->  bit_set_pat_1 ));
  iowrite32((uint32_t)(message.seq_q >>32)                , (volatile void*)(&slave_regs ->  bit_set_pat_2 ));
  iowrite32(message.n_seq_q                               , (volatile void*)(&slave_regs ->  nseqp         ));
  iowrite32((uint32_t)(message.length_seq_q & 0xFFFFFFFF) , (volatile void*)(&slave_regs ->  length_pat_1  ));
  iowrite32((uint32_t)(message.length_seq_q >>32)         , (volatile void*)(&slave_regs ->  length_pat_2  ));
  // Output
  iowrite32((uint32_t)(message.min_pos & 0xFFFFFFFF)      , (volatile void*)(&slave_regs ->  output_1      ));
  iowrite32((uint32_t)(message.min_pos >>32)              , (volatile void*)(&slave_regs ->  output_2      ));
  wait_type = message.wait_type;

  // pr_info("SEQ_DRIVER: Performed WRITE operation successfully\n");
  return 0;
}


// Set up the char_dev structure for this device.
static void seq_setup_cdev(struct seq_info *_seq_mem)
{
	int err, devno = MKDEV(seq_major, seq_minor);

	cdev_init(&_seq_mem->cdev, &seq_fops);
	_seq_mem->cdev.owner = THIS_MODULE;
	_seq_mem->cdev.ops = &seq_fops;
	err = cdev_add(&_seq_mem->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		pr_err("SEQ_DRIVER: Error %d adding seq cdev_add", err);

  pr_info("SEQ_DRIVER: Cdev initialized. Major number: %d, Minor number: %d\n", seq_major, seq_minor);
}


// The init function registers the chdev.
// It allocates dynamically a new major number.
// The major number corresponds to a different function driver.
static int seq_init(void)
{
  int result = 0;
  dev_t dev = 0;

  // Allocate a function number for our driver (major number).
  // The minor number is the instance of the driver.
  pr_info("SEQ_DRIVER: Allocating a new major number.\n");
  result = alloc_chrdev_region(&dev, seq_minor, 1, "seq");
  seq_major = MAJOR(dev);
  if (result < 0) {
    pr_err("SEQ_DRIVER: Can't get major %d\n", seq_major);
    return result;
  }

  // Request (exclusive) access to the memory address range of the peripheral.
  if (!request_mem_region(seq_mem.memStart, seq_mem.memEnd - seq_mem.memStart + 1, DRIVER_NAME)) {
    pr_err("SEQ_DRIVER: Couldn't lock memory region at %p\n", (void *)seq_mem.memStart);
    unregister_chrdev_region(dev, 1);
    return -1;
  }

  // Obtain a "kernel virtual address" for the physical address of the peripheral.
  seq_mem.baseAddr = ioremap(seq_mem.memStart, seq_mem.memEnd - seq_mem.memStart + 1);
  if (!seq_mem.baseAddr) {
    pr_err("SEQ_DRIVER: Could not obtain virtual kernel address for iomem space.\n");
    release_mem_region(seq_mem.memStart, seq_mem.memEnd - seq_mem.memStart + 1);
    unregister_chrdev_region(dev, 1);
    return -1;
  }

  init_waitqueue_head(&wq);

  // Request registering our interrupt handler for the IRQ of the peripheral.
  // We configure the interrupt to be detected on the rising edge of the signal.
  result = request_irq(seq_mem.irq, (irq_handler_t)seqIRQHandler, IRQF_TRIGGER_RISING, DRIVER_NAME, &seq_mem);
  if(result) {
    printk(KERN_ALERT "SEQ_DRIVER: Failed to register interrupt handler (error=%d)\n", result);     
    iounmap(seq_mem.baseAddr);
    release_mem_region(seq_mem.memStart, seq_mem.memEnd - seq_mem.memStart + 1);
    cdev_del(&seq_mem.cdev);
    unregister_chrdev_region(dev, 1);
    return result;
  }

  // Enable the IRQ. From this moment on, we can receive the IRQ asynchronously at any time.
  enable_irq(seq_mem.irq);
  pr_info("SEQ_DRIVER: Interrupt %d registered\n", seq_mem.irq);

  pr_info("SEQ_DRIVER: driver at 0x%08llX mapped to 0x%08llX\n", (uint64_t)seq_mem.memStart, (uint64_t)seq_mem.baseAddr); 
  seq_setup_cdev(&seq_mem);

  return 0;
}


// The exit function calls the cleanup
static void seq_exit(void)
{
	  pr_info("SEQ_DRIVER: calling cleanup function.\n");
	  seq_cleanup_module();
}

// Declare init and exit handlers.
// They are invoked when the driver is loaded or unloaded.
module_init(seq_init);
module_exit(seq_exit);


// The interrupt handler is called on the (rising edge of the) accelerator interrupt.
// The interrupt handler is executed in an interrupt context, not a process context!!!
// It must be quick, it cannot sleep. It cannot use functions that can sleep
// (e.g., don't allocate memory if that may wait for swapping).
// The handler cannot communicate directly with the user-space. The user-space does not
// interact with the interrupt handler.
static irq_handler_t seqIRQHandler(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
  volatile struct TRegs * slave_regs = (struct TRegs*)seq_mem.baseAddr;
  // Clean the interrupt in the peripheral, so that we can detect new rising transition.
  // The ISR is toggle-on-write (TOW), which means that its bits toggle when they are
  // written, whatever it was their previous value. Therefore, we write (1) to the 
  // 'done' bit to toggle it, so that it becomes 0 and the interrupt is disarmed.
  iowrite32(1, (volatile void*)&slave_regs->isr);
  mb();

  // Signal that it is us waking the main thread.
	flag = 1;
  // Wake the main thread.
	wake_up_interruptible(&wq);
	return (irq_handler_t) IRQ_HANDLED;      // Announce that the IRQ has been handled correctly
  // In case of error, or if it was not our device which generated the IRQ, return IRQ_NONE.
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ruben Rodriguez Alvarez <ruben.rodriguezalvarez@epfl.ch>");
MODULE_DESCRIPTION("Example device driver for controlling ULTRASCALE+ genome alignment");
MODULE_VERSION("1.0");



