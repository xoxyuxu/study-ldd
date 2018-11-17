#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

static int hello_proc_show(struct seq_file *m, void *v) {
    seq_printf(m, "Hello proc!\n");
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

static int __init hello_init(void)
{
	printk(KERN_INFO "init_module() called\n");
	proc_create("hello_proc", 0, NULL, &hello_proc_fops);
	return 0;
}

static void __exit hello_exit(void)
{
	printk(KERN_INFO "cleanup_module() called\n");
	remove_proc_entry("hello_proc", NULL);
}

MODULE_LICENSE("GPLv2");
module_init(hello_init);
module_exit(hello_exit);

/*
https://www.apriorit.com/dev-blog/195-simple-driver-for-linux-os
https://stackoverflow.com/questions/8516021/proc-create-example-for-kernel-module
*/

