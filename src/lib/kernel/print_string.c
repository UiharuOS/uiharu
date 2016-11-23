#include "print.h"

void print_string(char* string) {
    unsigned int i = 0;
    for(i = 0; string[i] != '\0'; ++i) {
        print_char(string[i]);
    }
}
