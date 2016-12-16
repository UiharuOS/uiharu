/***
 * main.c
 * ``````
 *   uiharu kernel main
 */

#include "print.h"
#include "init.h"
#include "debug.h"

void main(void) {
    print_string("This is uiharu kernel\n");
    init_all();
    // asm volatile ("sti");  // 打开中断
    // ASSERT(1==2);
    while(1); // 操作系统就是一个中断(事件)驱动的死循环(loop)
}
