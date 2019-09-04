/*
 * キャラクタデバイスの生成、クラス登録、デバイス登録で以下のファイルが生成される。
 * マイナーバージョンを複数にして、割り込み要因毎にデバイスを作成すれば
 * 複数のファイルを生成できる。オープンで file->priv にどこを開けたのかを残しておけば、
 * 選択できるね..
 *
 * /sys/cls_hello/dev_hello/{dev, power, subsystem,uevent}
 * /dev/dev_hello
 */

#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/kernel.h>

pid_t gt;

static int hello_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "Hello debugfs! write from %d\n", (int)gt );
	return 0;
}

static int hello_proc_open(struct inode *inode, struct  file *file)
{
	return single_open(file, hello_proc_show, NULL);
}


ssize_t my_write(struct file *file, const char __user *buff, size_t count, loff_t *offset)
{

	gt = current->pid;

	return (size_t)-ENOSYS;
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = hello_proc_open,
	.read = seq_read,
	.write = my_write,
	.llseek = seq_lseek,
	.release = single_release,
};

/* save directory entry pointer */
struct dentry* g_dentry;

static struct class* class;
static struct cdev cdev;
static struct device* pdev;
static dev_t devt;

static int __init hello_init(void)
{
int err, devno;

	printk(KERN_INFO "init_module() called\n");
	err = alloc_chrdev_region(&devt, 0 /*miner base*/, 1, "hello-cdev");
	if (err)
		return -ENODEV;

	devno = MKDEV(MAJOR(devt), 0);
	cdev_init( &cdev, &fops );

	// register 
	err = cdev_add( &cdev, devno, 1 );
	if (err) {
		printk(KERN_ERR "cdev_add() failed.\n");
		goto err_register;
	}
	class = class_create(THIS_MODULE, "cls_hello");
	if (!class) {
		printk(KERN_ERR "class_create() failed.\n");
		goto err_class;
	}
	pdev = device_create(class, NULL, devt, NULL, "dev_hello");
	if (!pdev) {
		printk(KERN_ERR "device_create() failed.\n");
		goto err_device;
	}
	
	return 0;
err_device:
	class_destroy( class );
err_class:
	cdev_del( &cdev );
err_register:
	unregister_chrdev_region(devt,1);
}

static void __exit hello_exit(void)
{
	int devno;

	printk(KERN_INFO "cleanup_module() called\n");
	devno = MKDEV(MAJOR(devt), 0);

	device_destroy( class, devt );
	class_destroy( class );
	cdev_del( &cdev );
	unregister_chrdev_region(devt,1);
}

MODULE_LICENSE("GPL");
module_init(hello_init);
module_exit(hello_exit);
/*
https://www.apriorit.com/dev-blog/195-simple-driver-for-linux-os
https://stackoverflow.com/questions/8516021/proc-create-example-for-kernel-module
*/
