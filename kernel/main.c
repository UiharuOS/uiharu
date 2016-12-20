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

    intr_disable();
    // create threads
    thread_start("k_thread_a", 5, k_thread_a, "misaka ");
    thread_start("k_thread_b", 4, k_thread_b, "kuroko ");
    // main thread
    while (1) {
        print_string("Main ");
    }
    intr_enable();

    return 0;        
}

void k_thread_a(void* arg) {
    /* thread run in kernel */
    char* para = arg;  // cast
    while (1) {
        // print_string(para);
    }
    // print_string(para);
}

void k_thread_b(void* arg) {
    /* thread run in kernel */
    char* para = arg;  // cast
    while (1) {
        print_string(para);
    }
    // print_string(para);
}
