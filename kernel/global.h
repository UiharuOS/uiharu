#ifndef _FUCK_CCNU
#define _FUCK_CCNU
#include "stdint.h"

/* 请求特权级定义 */
#define RPL0 0
#define RPL1 1
#define RPL2 2
#define RPL3 3

/* 描述符表位(该描述符是在gdt还是ldt中) */
#define TI_GDT 0
#define TI_LDT 1

/* 段描述符选择子定义 */
// 12位索引值(低) + TI + RPL(2位, 高)
#define SELECTOR_K_CODE  ((1<<3)+(TI_GDT<<2)+RPL0)
#define SELECTOR_K_DATA  ((2<<3)+(TI_GDT<<2)+RPL0)
#define SELECTOR_K_STACK ((3<<3)+(TI_GDT<<2)+RPL0)

/* IDT描述符8位属性定义 */
#define IDT_DESC_P 1
#define IDT_DESC_DPL0 0
#define IDT_DESC_DPL3 3
#define IDT_DESC_32_TYPE 0xE
#define IDT_DESC_16_TYPE 0x6

#define IDT_DESC_ATTR_DPL0 \
        ((IDT_DESC_P<<7)+(IDT_DESC_DPL0<<5)+(IDT_DESC_32_TYPE))
#define IDT_DESC_ATTR_DPL3 \
        ((IDT_DESC_P<<7)+(IDT_DESC_DPL3<<5)+(IDT_DESC_32_TYPE))

#endif
