#include "print.h"

void print_string(char string[], unsigned long size) {
    unsigned long i = 0;
    for(i = 0; i < size; ++i) {
        print_char(string[i]);
    }
    print_char('\n');
}
