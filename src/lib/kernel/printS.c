#include "print.h"

void print_string(char[]* string, uint64_t size) {
    for(uint64_t i = 0; i < size; ++i) {
        print_char(string[i]);
    }
    print_char('\n');
}
