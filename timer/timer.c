#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/printk.h>

struct timer_list mytimer;

static void my_callback(unsigned long unused)
{
	printk("Foo!!!\n");
}

static int __init temp_init_module(void)
{
	init_timer(&mytimer);
	mytimer.function = my_callback;
	mytimer.expires = jiffies + 10*HZ;
	/* unused */
	mytimer.data = 0;
	add_timer(&mytimer);
	return 0;
}

static void __exit temp_exit_module(void)
{
	printk("Goodbye\n");
}

module_init(temp_init_module);
module_exit(temp_exit_module);
MODULE_AUTHOR("yilun");
MODULE_LICENSE("GPL");
