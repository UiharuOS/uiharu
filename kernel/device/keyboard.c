#include "keyboard.h"
#include "print.h"
#include "interrupt.h"
#include "io.h"
#include "global.h"
#include "stdint.h"
#include "type.h"

/* 键盘缓冲寄存器端口号 */
#define KEYBOARD_BUFFER_PORT 0x60
/* 部分控制字符定义(转义字符) */
#define esc '\033'
#define backspace '\b'
#define tab '\t'
#define enter '\r'
#define delete '\177'

#define          char_invisible 0
#define ctrl_l   char_invisible
#define ctrl_r   char_invisible
#define shift_l  char_invisible
#define shift_r  char_invisible
#define alt_l    char_invisible
#define alt_r    char_invisible
#define capslock char_invisible
/* 控制字符的通码和断码
 * 定义通码即可, 第一套scancodeset: 断码=0x80+通码*/
#define shift_l_makecode  0x20
#define shift_r_makecode  0x36
#define alt_l_makecode    0x38
#define alt_r_makecode    0xe038
#define alt_r_breakcode   0xe0b8  // 扩展0xe0
#define ctrl_l_makecode   0x1d
#define ctrl_r_makecode   0xe01d
#define ctrl_r_breakcode  0xe09d  // 扩展0xe0
#define capslock_makecode 0x3a
/* 纪录控制字符的按键状态用于组合键 */
static boolean ctrl_status, shift_status, alt_status, capslock_status;
/* 纪录扩展按键(扫描码以0xe0开头) */
static boolean ext_scancode;
/* 二维数组映射通码(索引)->Ascii码 */
static char keymap[][2] = {
/* -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- 
  ██╗  ██╗███████╗██╗   ██╗██████╗  ██████╗  █████╗ ██████╗ ██████╗     ███╗   ███╗ █████╗ ██████╗ 
  ██║ ██╔╝██╔════╝╚██╗ ██╔╝██╔══██╗██╔═══██╗██╔══██╗██╔══██╗██╔══██╗    ████╗ ████║██╔══██╗██╔══██╗
  █████╔╝ █████╗   ╚████╔╝ ██████╔╝██║   ██║███████║██████╔╝██║  ██║    ██╔████╔██║███████║██████╔╝
  ██╔═██╗ ██╔══╝    ╚██╔╝  ██╔══██╗██║   ██║██╔══██║██╔══██╗██║  ██║    ██║╚██╔╝██║██╔══██║██╔═══╝ 
  ██║  ██╗███████╗   ██║   ██████╔╝╚██████╔╝██║  ██║██║  ██║██████╔╝    ██║ ╚═╝ ██║██║  ██║██║     
  ╚═╝  ╚═╝╚══════╝   ╚═╝   ╚═════╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚═════╝     ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝     
-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- */    
   /* 0x00 */{0, 0},     /* 0x01 */{esc, esc}, /* 0x02 */{'1', '!'}, /* 0x03 */{'2', '@'},
   /* 0x04 */{'3', '#'}, /* 0x05 */{'4', '$'}, /* 0x06 */{'5', '%'}, /* 0x07 */{'6', '^'},
   /* 0x08 */{'7', '&'}, /* 0x09 */{'8', '*'}, /* 0x0a */{'9', '('}, /* 0x0b */{'0', ')'},
   /* 0x0c */{'-', '_'}, /* 0x0d */{'=', '+'}, /* 0x0e */{backspace, backspace},
   /* 0x0f */{tab, tab}, /* 0x10 */{'q', 'Q'}, /* 0x11 */{'w', 'W'}, /* 0x12 */{'e', 'E'},
   /* 0x13 */{'r', 'R'}, /* 0x14 */{'t', 'T'}, /* 0x15 */{'y', 'Y'}, /* 0x16 */{'u', 'U'},
   /* 0x17 */{'i', 'I'}, /* 0x18 */{'o', 'O'}, /* 0x19 */{'p', 'P'}, /* 0x1a */{'[', '{'},
   /* 0x1b */{']', '}'}, /* 0x1c */{enter, enter}, /* 0x1d */{ctrl_l, ctrl_l},
   /* 0x1e */{'a', 'A'}, /* 0x1f */{'s', 'S'}, /* 0x20 */{'d', 'D'}, /* 0x21 */{'f', 'F'},
   /* 0x22 */{'g', 'G'}, /* 0x23 */{'h', 'H'}, /* 0x24 */{'j', 'J'}, /* 0x25 */{'k', 'K'},
   /* 0x26 */{'l', 'L'}, /* 0x27 */{';', ':'}, /* 0x28 */{'\'', '"'},/* 0x29 */{'`', '~'},
   /* 0x2a */{shift_l, shift_l},               /* 0x2b */{'\\', '|'},/* 0x2c */{'z', 'Z'},
   /* 0x2d */{'x', 'X'}, /* 0x2e */{'c', 'C'}, /* 0x2f */{'v', 'V'}, /* 0x30 */{'b', 'B'},
   /* 0x31 */{'n', 'N'}, /* 0x32 */{'m', 'M'}, /* 0x33 */{',', '<'}, /* 0x34 */{'.', '>'},
   /* 0x35 */{'/', '?'}, /* 0x36 */{shift_r, shift_r},               /* 0x37 */{'*', '*'},
   /* 0x38 */{alt_l, alt_l},                   /* 0x39 */{' ', ' '}, /* 0x3a */{capslock, capslock}
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* */    
};

static void intr_keyboard_handler(void) {
    boolean ctrl_down_last = ctrl_status;
    boolean shift_down_last = shift_status;
    boolean capslock_down_last = capslock_status;
    boolean break_code;
    // 读取输入缓冲寄存器
    uint16_t scancode = inb(KEYBOARD_BUFFER_PORT);

    if (scancode == 0xe0) {
        // 接受的是扩展扫描码
        ext_scancode = true;
        return;  // 扩展扫描码是多字节, 终止当前中断处理, 继续读取下面的scancode
    }
    if (ext_scancode) {
        // 如果上次接受的是扩展scancode前缀, 则合并scancode
        scancode = ((0xe000) | scancode);
        ext_scancode = false;
    }
    // 拿到断码, 如果发送的是通码(按键还没松开), 则结果为0
    break_code = ((scancode & 0x0080) != 0);
    if (break_code) {
        uint16_t make_code = (scancode &= 0xff7f); // 拿到通码
        if        (make_code == ctrl_l || make_code == ctrl_r) {
            ctrl_status = false;  // 按键已经松开了
        } else if (make_code == shift_l || make_code == shift_r) {
            shift_status = false;
        } else if (make_code == alt_l || make_code == alt_r) {
            alt_status = false;
        } /* capslock松开后不关闭 */
        return;
    } else if ((scancode > 0x00 && scancode < 0x3b) || \
               (scancode == alt_r) || (scancode == ctrl_r)) { /* 拿到的是通码 */
        boolean shift = false; 
        // 在keyboard map中索引对应字符(是否与shift组合)
        if ((scancode < 0x0e)  || (scancode == 0x29) || \
            (scancode == 0x1a) || (scancode == 0x1b) || \
            (scancode == 0x2b) || (scancode == 0x27) || \
            (scancode == 0x28) || (scancode == 0x33) || \
            (scancode == 0x34) || (scancode == 0x35)  ) {
            if (shift_down_last) {
                shift = true;  // 同时按下shift键
            }
        } else { /* 字母键 */
            if        (shift_down_last && capslock_down_last) {
                shift = false;
            } else if (shift_down_last || capslock_down_last) {
                shift = true;
            } else {
                shift = false;
            }
        }
        uint8_t index = (scancode &= 0x00ff);  // 拿到通码
        char current = 0;
        if (shift) { current = keymap[index][1]; }
        else       { current = keymap[index][0]; }
        if (current) {
            print_char(current);
            return;
        }

        if        (scancode == ctrl_l || scancode == ctrl_r) {
            ctrl_status = true;
        } else if (scancode == shift_l || scancode == shift_r) {
            shift_status = true;
        } else if (scancode == alt_l || scancode == alt_r) {
            alt_status = true;
        } else if (scancode == capslock) {
            capslock_status = !capslock_status;
        } else {
            print_string("unknown key\n");
        }
    }
}

void keyboard_init() {
    print_string("Info)--> keyboard_init start\n");
    register_handler(0x21, intr_keyboard_handler);
    print_string("Info)--> keyboard_init done\n");
}
