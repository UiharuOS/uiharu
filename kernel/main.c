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
#include "interrupt.h"

void k_thread_a(void*);
void k_thread_b(void*);

int main(void) {
    print_string("This is uiharu kernel\n");
    init_all();
    // 开中断
    intr_enable();
    // create threads
    thread_start("k_thread_a", 5, k_thread_a, "misaka ");
    thread_start("k_thread_b", 4, k_thread_b, "koroku ");
    // main thread
    while (1) {
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
