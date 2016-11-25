#include "print.h"

void print_string(char* string) {
    /*
    unsigned int i = 0;
    for(i = 0; string[i] != '\0'; ++i) {
        print_char(string[i]);
    }
    */
    char* addr = 0;
    for(addr = string; *addr != '\0'; addr++) {
        print_char(*addr);
    }
}
