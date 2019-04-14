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
#include <linux/slab.h>

#define MINOR_COUNT 2
struct priv_data {
	int index;
	int fun;
};

static ssize_t show_index(struct device *dev, struct device_attribute *dev_attr, char * buf)
{
	struct priv_data* priv = dev_get_drvdata( dev );
return sprintf(buf, "index=%d: dev=%p, priv=%p, attr=%p\n", priv->index,  dev, priv, dev_attr);
}

static ssize_t show_fun(struct device *dev, struct device_attribute *dev_attr, char * buf)
{
	struct priv_data* priv = dev_get_drvdata( dev );
return sprintf(buf, "fun=%d: dev=%p, priv=%p, attr=%p\n", priv->fun, dev, priv, dev_attr);
}

static ssize_t store_fun(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct priv_data* priv = dev_get_drvdata( dev );
	int val;
	char*endp;
	val =  simple_strtol(buf, &endp, 10);
	if (buf!=endp)
		priv->fun = val;
	return (ssize_t)(endp - buf);
}

static DEVICE_ATTR(index, 0444, show_index, NULL);
static DEVICE_ATTR(fun, 0644, show_fun, store_fun);

static struct attribute *dev_attrs[] = {
	&dev_attr_index.attr,
	&dev_attr_fun.attr,
	NULL,
};

static struct attribute_group dev_attr_group = {
	.attrs = dev_attrs,
};

static const struct attribute_group *dev_attr_groups[] = {
	&dev_attr_group,
	NULL,
};

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

static void device_create_release(struct device *dev)
{
	pr_debug("device: '%s': %s\n", dev_name(dev), __func__);
	kfree(dev);
}

static int __init hello_init(void)
{
	int err;
	dev_t devno;
	struct priv_data* priv;

	printk(KERN_INFO "init_module() called\n");
	err = alloc_chrdev_region(&devt, 0 /*minor base*/, MINOR_COUNT /* count */, "hello-cdev");
	if (err)
		return -ENODEV;


	// register 
	devno = MKDEV(MAJOR(devt), MINOR(devt));
	cdev_init( &cdev, &fops );
	err = cdev_add( &cdev, devno, MINOR_COUNT );
	if (err) {
		printk(KERN_ERR "cdev_add() failed.\n");
		goto err_register;
	}
	class = class_create(THIS_MODULE, "cls_hello");
	if (!class) {
		printk(KERN_ERR "class_create() failed.\n");
		goto err_class;
	}
	
	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		//ENOMEM
		goto err_device;
	}
	priv->index = 3;
#if 1
	devno = MKDEV(MAJOR(devt), MINOR(devt));
	pdev = device_create_with_groups(class, NULL, devno, priv, dev_attr_groups, "dev_hello0");
	if (!pdev) {
		printk(KERN_ERR "device_create() failed.\n");
		goto err_device;
	}

#else
	{
		struct device* dev;
		dev = kzalloc(sizeof(*dev), GFP_KERNEL);
		if (!dev) {
			err = -ENOMEM;
			goto err_device;
		}

		device_initialize(dev);
		dev->devt = devt;
		dev->class = class;
		dev->parent = NULL;
		dev->groups = dev_attr_groups;
		dev->release = device_create_release;
		dev->id = 1;
		dev_set_drvdata(dev, priv);
		err = kobject_set_name(&dev->kobj, "dev_hello1");
		if (err)
			goto err_device;

		err = device_add(dev);
		if (err)
			goto err_device;

		pdev = dev;
	}
	if (!pdev) {
		printk(KERN_ERR "device_create() failed.\n");
		goto err_device;
	}
#endif


	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		//ENOMEM
		goto err_device;
	}
	priv-> index = 33;
	devno = MKDEV(MAJOR(devt), MINOR(devt)+1);
	pdev = device_create_with_groups(class, NULL, devno, priv, dev_attr_groups, "dev_hello1");
	if (!pdev) {
		printk(KERN_ERR "device_create() failed.\n");
		goto err_device;
	}

	return 0;
err_device:
	class_destroy( class );
err_class:
err_register:
	cdev_del( &cdev );
	unregister_chrdev_region(devt,MINOR_COUNT);
return;
}

static void __exit hello_exit(void)
{
	dev_t devno;

	printk(KERN_INFO "cleanup_module() called\n");
	devno = MKDEV(MAJOR(devt), MINOR(devt));
	device_destroy( class, devt );
	devno = MKDEV(MAJOR(devt), MINOR(devt)+1);
	device_destroy( class, devt );

	class_destroy( class );
	cdev_del( &cdev );
	unregister_chrdev_region(devt,MINOR_COUNT);
}

MODULE_LICENSE("GPL");
module_init(hello_init);
module_exit(hello_exit);
/*
https://www.apriorit.com/dev-blog/195-simple-driver-for-linux-os
https://stackoverflow.com/questions/8516021/proc-create-example-for-kernel-module
*/
