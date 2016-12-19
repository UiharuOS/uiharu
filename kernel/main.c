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
#include "interrupt.h"

void k_thread_a(void*);
void k_thread_b(void*);

int main(void) {
    print_string("This is uiharu kernel\n");
    init_all();
    thread_start("k_thread_a", 31, k_thread_a, "misaka ");
    // thread_start("k_thread_b", 8, k_thread_b, "kuroko ");
    // 开启时钟中断
    intr_enable();
    while (1) {
        // 操作系统就是一个中断(事件)驱动的死循环(loop)
        print_string("Main ");
    }
    return 0;        
}

void k_thread_a(void* arg) {
    /* thread run in kernel */
    char* para = arg;  // cast
    while (1) {
        print_string(para);
    }
}

void k_thread_b(void* arg) {
    /* thread run in kernel */
    char* para = arg;  // cast
    while (1) {
        print_string(para);
    }
}
