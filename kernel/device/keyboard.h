#ifndef KERNEL_DEVICE_KEYBOARD_H
#define KERNEL_DEVICE_KEYBOARD_H

static void intr_keyboard_handler(void);
void keyboard_init();
extern struct bufferqueue keyboard_buf;

#endif
