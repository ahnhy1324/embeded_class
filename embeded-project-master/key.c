#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/ioport.h>
#include <linux/cdev.h>
#include <linux/device.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Altera University Program, modified by skyun");
MODULE_DESCRIPTION("DE1SoC Pushbutton Iterrupt Handler");

#define LW_BRIDGE_BASE    0xFF200000
#define LW_BRIDGE_SPAN    0x00200000
#define KEY_BASE   0x50
#define INTMASK      0x8
#define EDGE      0xC
#define IRQ_KEYS   73
#define DEVICE_NAME "key"
#define base_lwFPGA   0xFF200000
#define len_lwFPGA   0x200000

#define KEY_DEVNAME   "key"
void *lwbridgebase;
void  *key_addr;
static void* mem_base;
unsigned int checked_key;
static dev_t dev_no;
static struct cdev key_cdev;
static struct class* cl;

static int led_open(struct inode* minode, struct file* mfile) {
   return 0;
}

static int led_release(struct inode* minode, struct file* mfile) {
   return 0;
}

static ssize_t led_write(struct file* file, const char __user* buf, size_t count, loff_t* f_pos) {
   return 0;
}

static ssize_t led_read(struct file* file, char __user* buf, size_t count, loff_t* f_pos) {
   copy_to_user((void*)buf, (void*)&checked_key, 4);
   checked_key = 0;
   return 4;
}
static struct file_operations key_fo = {
   .read = led_read,
   .write = led_write,
   .open = led_open,
   .release = led_release
};
irq_handler_t irq_handler(int irq, void *dev_id, struct pt_regs *regs) {
   checked_key = ioread32(key_addr + EDGE)%16;
   iowrite32(0xf, key_addr + EDGE);
   return (irq_handler_t) IRQ_HANDLED;
}

static int __init intitialize_pushbutton_handler(void) {
   lwbridgebase = ioremap_nocache(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
   key_addr = lwbridgebase + KEY_BASE;

   iowrite32(0xf, key_addr + INTMASK);
   iowrite32(0xf, key_addr + EDGE);
   if (alloc_chrdev_region(&dev_no, 0, 1, DEVICE_NAME) < 0) {
      printk(KERN_ERR "alloc_chrdev_region() error\n");
      return -1;
   }

   cdev_init(&key_cdev, &key_fo);

   if (cdev_add(&key_cdev, dev_no, 1) < 0) {
      printk(KERN_ERR "cdev_add() error\n");
      goto unreg_chrdev;
   }

   cl = class_create(THIS_MODULE, DEVICE_NAME);
   if (cl == NULL) {
      printk(KERN_ALERT "class_create() error\n");
      goto unreg_chrdev;
   }

   if (device_create(cl, NULL, dev_no, NULL, DEVICE_NAME) == NULL) {
      printk(KERN_ALERT"device_create error\n");
      goto unreg_class;
   }

   mem_base = ioremap_nocache(base_lwFPGA, len_lwFPGA);
   if (mem_base == NULL) {
      printk(KERN_ERR "ioremap_nocache() error\n");
      goto un_device;
   }
   printk("Device: %s MAJOR: %d %x\n", DEVICE_NAME, MAJOR(dev_no), dev_no);

   return request_irq(IRQ_KEYS, (irq_handler_t)irq_handler, IRQF_SHARED, "pushbutton_irq_handler", (void*)(irq_handler));


un_device:
   device_destroy(cl, dev_no);
unreg_class:
   class_destroy(cl);
unreg_chrdev:
   unregister_chrdev_region(dev_no, 1);
   return -1;
}

static void __exit cleanup_pushbutton_handler(void) {
   free_irq(IRQ_KEYS, (void *) irq_handler);
   device_destroy(cl, dev_no);
   class_destroy(cl);
   unregister_chrdev_region(dev_no, 1);
   unregister_chrdev_region(dev_no, 1);
   printk(" %s unregistered.\n", DEVICE_NAME);
}

module_init(intitialize_pushbutton_handler);
module_exit(cleanup_pushbutton_handler);
