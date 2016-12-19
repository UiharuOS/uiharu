#ifndef _KERNEL_INTERRUPT_H
#define _KERNEL_INTERRUPT_H

#include "stdint.h"

void idt_init(void);
typedef void* intr_handler;
// 定义中断的两种状态
enum intr_status {  // 枚举体
    INTR_OFF,       // 中断关闭
    INTR_ON         // 中断开启
};

enum intr_status intr_enable(void);
enum intr_status intr_disable(void);
enum intr_status intr_set_status(enum intr_status status);
enum intr_status intr_get_status(void);

intr_handler idt_table[IDT_DESC_CNT];  // 中断处理程序数组, 每个中断向量索引一个中断处理程序

#endif
