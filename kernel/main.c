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
    /***********  threads  **********/
    uthread_create("k_thread_a", 10, k_thread_a, "misaka ");
    uthread_create("k_thread_b", 8, k_thread_b, "kuroko ");
    /***********  threads  **********/
    // intr_enable();
    intr_set_status(INTR_ON);
    while(1) {
        print_string("Main ");
    };

    return 0;
}

void k_thread_a(void* arg) {     
    char* para = arg;  // cast
    while(1) {
        intr_disable();
        print_string(para);
        intr_enable();
    }
}

void k_thread_b(void* arg) {     
    char* para = arg;  // cast
    while(1) {
        intr_disable();
        print_string(para);
        intr_enable();
    }
}
