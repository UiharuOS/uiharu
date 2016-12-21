/***
 * main.c
 * ``````
 *   uiharu kernel main
 */
#include "print.h"
#include "init.h"
#include "thread.h"
#include "interrupt.h"

void k_thread_a(void*);
void k_thread_b(void*);

int main(void) {
    print_string("This is uiharu kernel\n");
    init_all();

    thread_start("k_thread_a", 10, k_thread_a, "misaka ");
    // -> why kuroko not in list?
    print_string("k_thread_b");
    thread_start("k_thread_b", 8, k_thread_b, "kuroko ");

    while(1) {
      print_string("Main ");
    };

    intr_enable();	// 打开中断,使时钟中断起作用

    return 0;
}

void k_thread_a(void* arg) {     
    char* para = arg;
    while(1) {
        intr_disable();
        print_string(para);
        intr_enable();
    }
}

void k_thread_b(void* arg) {     
    char* para = arg;
    while(1) {
        intr_disable();
        print_string(para);
        intr_enable();
    }
}
