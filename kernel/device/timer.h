#ifndef KERNEL_DEVICE_TIMER_H
#define KERNEL_DEVICE_TIMER_H

#include "stdint.h"

uint32_t ticks;

void time_init();
static void intr_timer_handler(void);

#endif
