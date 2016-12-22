#ifndef _KERNEL_INTERRUPT_H
#define _KERNEL_INTERRUPT_H

#include "stdint.h"
#define IDT_DESC_CNT 0x30  // 目前总共支持0x21(33)个中断向量

struct gate_desc {
    // 中断门描述符结构体
    uint16_t func_offset_low_word;  // 中断处理程序在目标代码段中的偏移量的15~0位
    uint16_t selector;              // 中断处理程序目标代码段描述符选择子
    uint8_t dcount;                 // 未使用以及固定为0的字段
    uint8_t attribute;              // 属性位(TYPE, S, DPL, P)
    uint16_t func_offset_high_word; // 中断处理程序在目标代码段内的偏移量的31~16位
};

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
void register_handler(uint8_t vector_no, intr_handler function);

intr_handler idt_table[IDT_DESC_CNT];  // 中断处理程序数组, 每个中断向量索引一个中断处理程序
static struct gate_desc idt[IDT_DESC_CNT];  // 中断描述符表
char* intr_name[IDT_DESC_CNT];         // 保存异常[名字](你的名字把uiharu写完看)
void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler function);

#endif
