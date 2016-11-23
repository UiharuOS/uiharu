#include "print.h"

void _dec_print(unsigned int integer) {
    unsigned int i = 0, count = 0;
    unsigned int _integers[10];
    for(i = 0; integer > 0; ++i) {
        _integers[i] = integer % 10;
        integer /= 10;
        count++;
    }
    for(i = count; i > 0; i--) {
        print_char(_integers[i-1] + 48);
    }
}

void _hex_print(unsigned int integer) {
    unsigned int i = 0, count = 0, _num = 0;
    unsigned int _integers[10];
    for(i = 0; integer > 0; ++i) {
        _integers[i] = integer % 16;
        integer /= 16;
        count++;
    }
    for(i = count; i > 0; i--) {
        _num = _integers[i-1];
        if      (_num == 0)  { print_char('0'); }
        else if (_num == 1)  { _dec_print(1);   }
        else if (_num == 2)  { _dec_print(2);   }
        else if (_num == 3)  { _dec_print(3);   }
        else if (_num == 4)  { _dec_print(4);   }
        else if (_num == 5)  { _dec_print(5);   }
        else if (_num == 6)  { _dec_print(6);   }
        else if (_num == 7)  { _dec_print(7);   }
        else if (_num == 8)  { _dec_print(8);   }
        else if (_num == 9)  { _dec_print(9);   }
        else if (_num == 10) { print_char('a'); }
        else if (_num == 11) { print_char('b'); }
        else if (_num == 12) { print_char('c'); }
        else if (_num == 13) { print_char('d'); }
        else if (_num == 14) { print_char('e'); }
        else if (_num == 15) { print_char('f'); }
    }
}

void _bin_print(unsigned int integer) {
    unsigned int i = 0, count = 0, _num = 0;
    unsigned int _integers[35];
    for(i = 0; integer > 0; ++i) {
        _integers[i] = integer % 2;
        integer /= 2;
        count++;
    }
    for(i = count; i > 0; i--) {
        _num = _integers[i-1];
        if      (_num == 0) { print_char('0'); }
        else if (_num == 1) { print_char('1'); }
    }

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
