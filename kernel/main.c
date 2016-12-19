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
#include "thread.h"

void k_thread_a(void*);

int main(void) {
    print_string("This is uiharu kernel\n");
    init_all();
    // asm volatile ("sti");  // 打开中断
    // ASSERT(1==2);
    // uint32_t* addr = (uint32_t*)get_kernel_pages(3);
    // uint32_t* _addr = (uint32_t*)get_kernel_pages(1);
    // page_table_add((void*)addr, (void*)0x1500);
    // print_string("\nTest)-> get_kernel_page start vaddr is ");
    // print_int(addr, 'H');
    // print_int(_addr, 'H');
    // print_string("\n");
    thread_start("k_thread_a", 31, k_thread_a, "neo1218 ");
    while(1); // 操作系统就是一个中断(事件)驱动的死循环(loop)
    return 0;        
}

void k_thread_a(void* arg) {
    /* thread run in kernel */
    char* para = arg;  // cast
    while(1) {
        print_string(para);
    }
}
