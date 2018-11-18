#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>

static int hello_proc_show(struct seq_file *m, void *v) {
	seq_printf(m, "Hello debugfs!\n");
	return 0;
}

static int hello_proc_open(struct inode *inode, struct  file *file) {
	return single_open(file, hello_proc_show, NULL);
}

static const struct file_operations hello_proc_fops = {
	.owner = THIS_MODULE,
	.open = hello_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

/* save directory entry pointer */
struct dentry* g_dentry;

static int __init hello_init(void)
{
	struct dentry* pd;

	printk(KERN_INFO "init_module() called\n");
	pd = debugfs_create_dir("hello_dir", NULL);
	if (pd) {
		g_dentry = debugfs_create_file("hello_proc", 0666, pd, NULL, &hello_proc_fops);
	}
	else {
		return 1;
	}
	return 0;
}

static void __exit hello_exit(void)
{
	printk(KERN_INFO "cleanup_module() called\n");
	debugfs_remove_recursive( g_dentry );
}

MODULE_LICENSE("GPL");
module_init(hello_init);
module_exit(hello_exit);

/*
https://www.apriorit.com/dev-blog/195-simple-driver-for-linux-os
https://stackoverflow.com/questions/8516021/proc-create-example-for-kernel-module
*/

