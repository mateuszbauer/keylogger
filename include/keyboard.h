#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <linux/keyboard.h>

void keyboard_init(void);

void keyboard_cleanup(void);

int key_pressed(struct notifier_block *nb, unsigned long action, void *data);

#endif // _KEYBOARD_H_
