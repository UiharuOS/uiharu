/***
 * init.c
 * ``````
 *   初始化模块调用
 */
#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "../device/timer.h"

void init_all(void) {
    /* init_all: 初始化所有模块
     */
    print_string("init all\n");
    idt_init();     // 初始化中断向量表(描述符表)
    timer_init();   // 初始化8253PIC, 提高时钟中断频率(100HZ/S)
}
