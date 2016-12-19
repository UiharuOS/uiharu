/***
 * main.c
 * ``````
 *   uiharu kernel main
 */

#include "print.h"
#include "init.h"
#include "debug.h"
#include "string.h"
#include "stdint.h"

int main(void) {
    print_string("This is uiharu kernel\n");
    init_all();
    // asm volatile ("sti");  // 打开中断
    // ASSERT(1==2);
    uint32_t* addr = (uint32_t*)get_kernel_pages(3);
    print_string("\nTest)-> get_kernel_page start vaddr is ");
    print_int(addr, 'H');
    print_string("\n");
    while(1); // 操作系统就是一个中断(事件)驱动的死循环(loop)
    return 0;        
}
