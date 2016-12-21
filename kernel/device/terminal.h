#ifndef KERNEL_DEVICE_TERMINAL_H
#define KERNEL_DEVICE_TERMINAL_H

#include "stdint.h"

void terminal_init();
void terminal_acquire();
void terminal_release();
void tprint_string(char* string);
void tprint_char(char ch);
void tprint_int(int integer, char mode);

#endif
