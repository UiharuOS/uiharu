/***
 * init.c
 * ``````
 *   初始化模块调用
 */
#include "init.h"
#include "print.h"
#include "interrupt.h"

void init_all(void) {
    /* init_all: 初始化所有模块
     */
    print_string("init all\n");
    idt_init();
}
