#ifndef _KERNEL_INTERRUPT_H
#define _KERNEL_INTERRUPT_H

#include "stdint.h"

void idt_init(void);
typedef void* intr_handler;

#endif
