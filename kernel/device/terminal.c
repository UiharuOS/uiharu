/***
 * terminal.c
 * ```````````
 *   终端, 标准输入输出设备
 */

#include "terminal.h"
#include "print.h"
#include "stdint.h"
#include "sync.h"
#include "thread.h"

static struct lock terminal_lock;  // 终端锁

void terminal_init() {
    /* terminal_init 初始化终端 */
    print_string("Info)--> terminal_init start\n");
    lock_init(&terminal_lock);
    print_string("Info)--> terminal_init done\n");
}

void terminal_acquire() {
    /* terminal_acquire 获取终端 */
    lacquire(&terminal_lock);
}

void terminal_release() {
    /* terminal_release 释放终端 */
    lrelease(&terminal_lock);
}

void tprint_string(char* string) {
    /* terminal_print_string 终端中输出字符串 */
    // 对显存的操作属于临界区, 需要上锁
    terminal_acquire();
    print_string(string);
    terminal_release();
}

void tprint_char(char ch) {
    /* terminal_print_char 终端中输出字符 */
    terminal_acquire();
    print_char(ch);
    terminal_release();
}

void tprint_int(int integer, char mode) {
    /* terminal_print_int 终端中输出数字 */
    terminal_acquire();
    print_int(integer, mode);
    terminal_release();
}
