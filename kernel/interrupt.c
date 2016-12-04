/***
 * interrupt.c
 * ```````````
 *   创建中断描述符表IDT, 安装中断处理程序
 *   IDT简单说就是中断向量号以及该向量对应的中断处理例程
 */
#include "interrupt.h"
#include "stdint.h"  // 定义各种数据类型
#include "global.h"

#define IDT_DESC_CNT 0x21  // 目前总共支持0x21(33)个中断向量

struct gate_desc {
    // 中断门描述符结构体
    uint16_t func_offset_low_word;  // 中断处理程序在目标代码段中的偏移量的15~0位
    uint16_t selector;              // 中断处理程序目标代码段描述符选择子
    uint8_t dcount;                 // 未使用以及固定为0的字段
    uint8_t attribute;              // 属性位(TYPE, S, DPL, P)
    uint16_t func_offset_high_word; // 中断处理程序在目标代码段内的偏移量的31~16位
}

// static 函数声明
static void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler function);
static struct gate_desc idt[IDT_DESC_CNT];
/* idt: 创建中断门描述符, idt本质上就是中断门描述符数组
 *  - IDT_DESC_CNT: 中断向量的个数
 */
// extern 外部变量
// 引用kernel.S中定义的中断向量数组, 对应相应的中断处理例程
extern intr_handler intr_entry_table[IDT_DESC_CNT];

static void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler function) {
    /* make_idt_desc: 创建中断门描述符
     *  - p_gdesc: 指向中断门描述符的指针
     *  - attr: 8位属性位
     *  - function: 中断处理程序(门描述符对应的代码段)
     */
    p_gdesc->func_offset_low_word = (uint32_t)function & 0x0000FFFF; // 中断处理函数的低16位
    p_gdesc->selector = SELECTOR_K_CODE; // 中断处理函数代码段选择子
    p_gdesc->dcount = 0; // 固定位, 置为0
    p_gdesc->attribute = attr; // 属性位
    p_gdesc->func_offset_high_word = ((uint32_t)function & 0xFFFF0000) >> 16; // 中断处理函数的高16位
}

static void idt_desc_init(void) {
    /* idt_desc_init: 初始化中断描述符
     */
    int i;
    for(i = 0; i < IDT_DESC_CNT; i++) {
        // intr_entry_table[i] 第i个中断向量对应的中断处理例程
        make_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]);
    }
    print_string("idt_desc_init done\n");
}

void idt_init() {
    /* idt_init: 初始化中断描述符表
     */
    print_string("idt_init start\n");
    idt_desc_init(); // 初始化中断描述符
    pic_init();      // 初始化中断代理芯片(ex:8259A)

    uint64_t idt_operand = (
        (sizeof(idt) - 1) | ((uint64_t)(
            (uint32_t)idt << 16)
        )
    );
    print_string("idt_init done\n");
}
