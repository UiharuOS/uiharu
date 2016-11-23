#include "print.h"

void _dec_print(unsigned int integer) {
    print_string("test");
}

void print_int(unsigned int integer, char mode) {
    if      (mode == 'D') {
        _dec_print(integer);
    }
    else if (mode == 'H') {
        _hex_print(integer);
    }
    else if (mode == 'B') {
        _bin_print(integer);
    }
}
