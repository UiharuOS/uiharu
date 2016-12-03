#include "print.h"  // lib/kernel/print.h
void main(void) {
    // this is a process
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
    print_int(1234567890, 'D');
    print_char('\n');
    print_string("Decimal ");
    print_int(57435, 'D');
    print_string("\nHexadecimal ");
    print_int(57435, 'H');
    print_string("\nBinary ");
    print_int(57435, 'B');
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
