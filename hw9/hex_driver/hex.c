#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <asm/io.h>

#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/ioport.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HeeYoung Ahn");
MODULE_DESCRIPTION("seven segment LEDs");

#define base_lwFPGA	0xFF200000
#define len_lwFPGA	0x200000

#define addr_HEX30	0x20
#define addr_HEX54	0x30
#define HEX_DEVMAJOR	240
#define HEX_DEVNAME	"hex"

static void *mem_base;
static void *hex30_addr;
static void *hex54_addr;

static int hex_conversions[16] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71};
static unsigned int hex_data = 0;

static int hex_open(struct inode *minode, struct file *mfile) {
	return 0;
}

static int hex_release(struct inode *minode, struct file *mfile) {
	return 0;
}

static ssize_t hex_write (struct file *file, const char __user *buf, size_t count, loff_t *f_pos) {
	unsigned int tmp_hex_data = 0;
	unsigned int hex30_data = 0;
	unsigned int hex54_data = 0;

	get_user(tmp_hex_data, (unsigned int *) buf);
	
	tmp_hex_data &= 0xFFFFFF;

	hex_data = tmp_hex_data;
	hex30_data |= hex_conversions[tmp_hex_data & 0xF];
	tmp_hex_data >>= 4;
	hex30_data |= hex_conversions[tmp_hex_data & 0xF] << 8;
	tmp_hex_data >>= 4;
	hex30_data |= hex_conversions[tmp_hex_data & 0xF] << 16;
	tmp_hex_data >>= 4;
	hex30_data |= hex_conversions[tmp_hex_data & 0xF] << 24;
	tmp_hex_data >>= 4;
	hex54_data |= hex_conversions[tmp_hex_data & 0xF];
	tmp_hex_data >>= 4;
	hex54_data |= hex_conversions[tmp_hex_data & 0xF] << 8;
	
	iowrite32(hex30_data, hex30_addr);
	iowrite32(hex54_data, hex54_addr);
	return count;
}

static ssize_t hex_read (struct file *file, char __user *buf, size_t count, loff_t *f_pos) {
	put_user(hex_data, (unsigned int *) buf);
	return 4;
}

static struct file_operations hex_fops = {
	.read = hex_read,
	.write = hex_write,
	.open = hex_open,
	.release = hex_release
};

static int __init hex_init(void) {
	int res;
	
	res = register_chrdev(HEX_DEVMAJOR, HEX_DEVNAME, &hex_fops);
	if (res < 0) {
		printk(KERN_ERR " hexs : failed to register device.\n");
		return res;
	}

	mem_base = ioremap_nocache(base_lwFPGA, len_lwFPGA);
	if (!mem_base) {
		printk("Error mapping memory\n");
		release_mem_region(base_lwFPGA, len_lwFPGA);
		return -EBUSY;
	}
	hex30_addr = mem_base + addr_HEX30;
	hex54_addr = mem_base + addr_HEX54;
	
	printk("Device: %s MAJOR: %d\n", HEX_DEVNAME, HEX_DEVMAJOR);
	return 0;
}

static void __exit hex_exit(void) {
	unregister_chrdev(HEX_DEVMAJOR, HEX_DEVNAME);
	printk(" %s unregistered.\n", HEX_DEVNAME);
	iounmap(mem_base);
}

module_init(hex_init);
module_exit(hex_exit);
