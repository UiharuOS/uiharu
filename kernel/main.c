/***
 * main.c
 * ``````
 *   uiharu kernel main
 */

#include "print.h"
#include "init.h"
void main(void) {
    print_string("I am kernel\n");
    init_all();
    asm volatile ("sti");  // 打开中断
    print_string("hq, this is uiharu");
    while(1);
}
