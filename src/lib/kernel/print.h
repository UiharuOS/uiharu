#ifdef _FUCK_CCNU_LIB_KERNEL_PRINT_H
#define _FUCK_CCNU_LIB_KERNEL_PRINT_H

#include "stdint.h"
void print_char(uint8_t ascii_char);    // 打印单个字符
void print_string(char* string);        // 打印字符串(长度无限制)
void print_str(char* string);           // 打印字符串(长度无限制)

#endif
