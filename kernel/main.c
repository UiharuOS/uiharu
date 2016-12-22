/***
 * main.c
 * ``````
 *   uiharu kernel main
 */
#include "print.h"
#include "init.h"
#include "thread.h"
#include "interrupt.h"
#include "terminal.h"
#include "keyboard.h"
#include "bufferqueue.h"

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
    //    tprint_string("Main ");
    };

    return 0;
}

void k_thread_a(void* arg) {     
    while(1) {
        // tprint_string(para);
        enum intr_status old_status = intr_disable();
        if (!bufferqueue_empty(&keyboard_buf)) {
            tprint_string(arg);
            char byte = bufferqueue_getchar(&keyboard_buf);
            tprint_char(byte);
        }
        intr_set_status(old_status);
    }
}

void k_thread_b(void* arg) {     
    while(1) {
        // tprint_string(para);
        enum intr_status old_status = intr_disable();
        if (!bufferqueue_empty(&keyboard_buf)) {
            tprint_string(arg);
            char byte = bufferqueue_getchar(&keyboard_buf);
            tprint_char(byte);
        }
        intr_set_status(old_status);
    }
}
