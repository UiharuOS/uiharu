#include "keyboard.h"
#include "print.h"
#include "interrupt.h"
#include "io.h"
#include "global.h"

#define KEYBOARD_BUFFER_PORT 0x60  // 键盘缓冲寄存器端口号

static void intr_keyboard_handler(void) {
    print_char('k');
    inb(KEYBOARD_BUFFER_PORT); // 读取输入缓冲寄存器
    return;
}

void keyboard_init() {
    print_string("Info)--> keyboard_init start\n");
    register_handler(0x21, intr_keyboard_handler);
    print_string("Info)--> keyboard_init done\n");
}
