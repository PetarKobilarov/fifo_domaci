#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#define	BUFF_SIZE 176

MODULE_LICENSE("Dual BSD/GPL");

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;

int fifo[16];
int pos = 0;
int endRead = 0;
int n = 1;

int fifo_open(struct inode *pinode, struct file *pfile);
int fifo_close(struct inode *pinode, struct file *pfile);
ssize_t fifo_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t fifo_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

struct file_operations my_fops = 
{
	.owner = THIS_MODULE,
	.open = fifo_open,
	.read = fifo_read,
	.write = fifo_write,
	.release = fifo_close,
};

int fifo_open(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully opened fifo\n");
		return 0;
}

int fifo_close(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully closed fifo\n");
		return 0;
}

ssize_t fifo_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset)
{
	int ret, i = 0, j = 0;
	char buff[BUFF_SIZE];
	long int len = 0;
	if(endRead)
	{
		endRead = 0;
		return 0;
	}

	if(pos > 0)
	{
		if(n <= pos)
		{
			for(j = 0; j < n; j++)
			{
//				len = scnprintf(buff, BUFF_SIZE, "%d ", fifo[0]);
				
				buff[j] = fifo[0];

				for(i = 0; i < pos; i++)
 					fifo[i] = fifo[i+1];
		
			}
			len = n;
			pos -= n;
				ret = copy_to_user(buffer, buff, len);
				if(ret)
					return -EFAULT;
			printk(KERN_INFO "Succesfully read\n");
			endRead = 1;
		}else
		{
			printk(KERN_INFO "Not enough values in Fifo\n");
			endRead = 1;
		}
//		endRead = 1;
	}
	else
	{
			printk(KERN_WARNING "Fifo is empty\n"); 
	}
	
	return len;
}

ssize_t fifo_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset)
{
	char buff[BUFF_SIZE], binNums[16][8];
	int value = 0;
	int ret;
	int i = 0, j = 0, k = 0;

	ret = copy_from_user(buff, buffer, length);
	if(ret)
		return -EFAULT;
	
	buff[length-1] = '\0';

	if(buff[0] == 'n' && buff[1] == 'u' && buff[2] == 'm' && buff[3] == '=')
		{

			n = buff[4] - 48;
			printk(KERN_INFO "Succesfully set to read %d numbers at once", n);
			/*ret = sscanf(buff, "%d", &n);
			if(ret == 1)
				printk(KERN_INFO "Succesfully set to read %d numbers at once", n);
			else
				printk(KERN_WARNING "Wrong command format\n");*/
		}
	
	if(pos < 16)
	{
			for(i = 0; i < length-1; i++)
			{
				if(buff[i-1] == 'b')
				{
					for(k = 0; k < 8; k++)
						binNums[j][k] = buff[i+k];

					j++;
				}
			}

			for(i = 0; i < j; i++)
			{
				if(binNums[i][7] == '1')
					value += 1;
				if(binNums[i][6] == '1')
					value += 2;
				if(binNums[i][5] == '1')
					value += 4;
				if(binNums[i][4] == '1')
					value += 8;
				if(binNums[i][3] == '1')
					value += 16;
				if(binNums[i][2] == '1')
					value += 32;
				if(binNums[i][1] == '1')
					value += 64;
				if(binNums[i][0] == '1')
					value += 128;

				printk(KERN_INFO "Succesfully wrote value %d\n", value);
				fifo[pos] = value;
				value = 0;
				pos++;
			}

			
		
	}

	return length;
}

static int __init fifo_init(void)
{
   int ret = 0;
	int i=0;

	//Initialize array
	for (i=0; i<10; i++)
		fifo[i] = 0;

   ret = alloc_chrdev_region(&my_dev_id, 0, 1, "fifo");
   if (ret)
   {
      printk(KERN_ERR "failed to register char device\n");
      return ret;
   }
   printk(KERN_INFO "char device region allocated\n");

   my_class = class_create(THIS_MODULE, "fifo_class");
   if (my_class == NULL)
   {
      printk(KERN_ERR "failed to create class\n");
      goto fail_0;
   }
   printk(KERN_INFO "class created\n");
   
   my_device = device_create(my_class, NULL, my_dev_id, NULL, "fifo");
   if (my_device == NULL)
   {
      printk(KERN_ERR "failed to create device\n");
      goto fail_1;
   }
   printk(KERN_INFO "device created\n");

	my_cdev = cdev_alloc();	
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;
	ret = cdev_add(my_cdev, my_dev_id, 1);
	if (ret)
	{
      printk(KERN_ERR "failed to add cdev\n");
		goto fail_2;
	}
   printk(KERN_INFO "cdev added\n");
   printk(KERN_INFO "Hello world\n");

   return 0;

   fail_2:
      device_destroy(my_class, my_dev_id);
   fail_1:
      class_destroy(my_class);
   fail_0:
      unregister_chrdev_region(my_dev_id, 1);
   return -1;
}

static void __exit fifo_exit(void)
{
   cdev_del(my_cdev);
   device_destroy(my_class, my_dev_id);
   class_destroy(my_class);
   unregister_chrdev_region(my_dev_id,1);
   printk(KERN_INFO "Goodbye, cruel world\n");
}

module_init(fifo_init);
module_exit(fifo_exit);
