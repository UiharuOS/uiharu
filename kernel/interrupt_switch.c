/***
 * flags(16位) -> eflags(32位)
 *   -> 0x00000200(32) 0000 0000 0000 0000 0000 0010 0000 0000
 *   -> IF: interrupt enable flag
 */
#include "interrupt.h"

#define EFLAGS_IF 0x00000200  // eflags寄存器if位为1
#define GET_EFLAGS(EFLAGS_VAR) asm volatile ("pushfl; popl %0" : "=g" (EFLAGS_VAR))

enum intr_status intr_enable() {
    /* intr_enable 开中断并返回开中断前的状态
     */
    enum intr_status old_status;
    if (INTR_ON == intr_get_status()) {
        old_status = INTR_ON;
        return old_status;
    } else {
        old_status = INTR_OFF;
        asm volatile ("sti");   // 开中断, sti指令会将eflags的if位置为1
        return old_status;
    }
}

enum intr_status intr_disable() {
    /* intr_disable: 关中断并返回相应状态
     */
    enum intr_status old_status;
    if (INTR_ON == intr_get_status()) {
        old_status = INTR_ON;
        asm volatile ("cli" : : : "memory"); // 关中断, cli将eflags的if位置为0
        return old_status;
    }
    else {
        old_status = INTR_OFF;
        return old_status;
    }
}

enum intr_status intr_set_status(enum intr_status status) {
    /* intr_set_status: 设置中断状态
     *   - status: 待设置的中断状态
     */
    // 中断状态比较简单, 不是开就是关
    return status & INTR_ON ? intr_enable() : intr_disable();
}

enum intr_status intr_get_status() {
    /* intr_get_status: 获取当前中断状态
     *   根据eflags寄存器的值获取
     */
    uint32_t eflags = 0;
    GET_EFLAGS(eflags);
    // 为true eflags寄存器的值为1 -> INTR_ON
    // 判断eflags的第10位(IF位)是否为1
    return (EFLAGS_IF & eflags) ? INTR_ON : INTR_OFF;
}
