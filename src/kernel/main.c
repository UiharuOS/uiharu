#include "print.h"  // lib/kernel/print.h
void main(void) {
    /* kernel main */
    /*
    print_char('u');
    print_char('i');
    print_char('h');
    print_char('a');
    print_char('r');
    print_char('u');
    */
    print_string("---------  test int --------\n");
    print_char(48);  // ascii
    print_string("\n---------  test str --------\n");
    print_string("I love uiharu\nuiharu\n");
    print_char('O');
    print_char('S');
    //print_char('\b');
    //print_char('S');
    print_char('\r');
    print_str("misaka ");
    while(1);
}
