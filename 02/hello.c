#include <linux/module.h>
#include <linux/kernel.h>
 
int hello_init(void)
{
	printk(KERN_INFO "hello_init() called\n");
	return 0;
}
 
void hello_exit(void)
{
	printk(KERN_INFO "hello_cleanup() called\n");
}

MODULE_LICENSE("GPLv2");
module_init(hello_init);
module_exit(hello_exit);

/*
 * module_init():
 *   #define module_init(x)  __initcall(x);
 *
 * modle_exit():
 *   #define module_exit(x)  __exitcall(x);
 *
 * __initcall():
 *   #define __initcall(fn) device_initcall(fn)
 *
 * FILE: linux/init.h
 *  #define device_initcall(fn)             __define_initcall(fn, 6)
 *	#define __define_initcall(fn, id) \
 *			static initcall_t __initcall_##fn##id __used \
 *			__attribute__((__section__(".initcall" #id ".init"))) = fn;
 */
