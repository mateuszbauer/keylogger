#ifndef _SYS_CALLS_H_
#define _SYS_CALLS_H_

#include <linux/syscalls.h>

int sys_calls_init(void);

void sys_calls_cleanup(void);

#endif // _SYS_CALLS_H_
