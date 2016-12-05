/***
 * interrupt.c
 * ```````````
 *   创建中断描述符表IDT, 安装中断处理程序
 *   IDT简单说就是中断向量号以及该向量对应的中断处理例程
 */
#include "interrupt.h"
#include "stdint.h"  // 定义各种数据类型
#include "global.h"
#include "io.h"

#define IDT_DESC_CNT 0x21  // 目前总共支持0x21(33)个中断向量
#define PIC_M_CTRL 0x20    // PIC主片控制端口号 
#define PIC_M_DATA 0x21    // PIC主片数据端口号
#define PIC_S_CTRL 0xa0    // PIC从片控制端口号
#define PIC_S_DATA 0xa1    // PIC从片数据端口号

struct gate_desc {
    // 中断门描述符结构体
    uint16_t func_offset_low_word;  // 中断处理程序在目标代码段中的偏移量的15~0位
    uint16_t selector;              // 中断处理程序目标代码段描述符选择子
    uint8_t dcount;                 // 未使用以及固定为0的字段
    uint8_t attribute;              // 属性位(TYPE, S, DPL, P)
    uint16_t func_offset_high_word; // 中断处理程序在目标代码段内的偏移量的31~16位
};

// static 函数声明
void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler function);
static struct gate_desc idt[IDT_DESC_CNT];
// extern 外部对象声明
extern intr_handler intr_entry_table[IDT_DESC_CNT];

void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler function) {
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
    print_string("idt desc init done\n");
}

static void pic_init(void) {
    /* pic_init: 初始化8259A PIC芯片
     */
    // 初始化主片
    // 往主片中写入ICW1(设置工作模式: 边沿触发、级联)
    outb (PIC_M_CTRL, 0x11); // ic4为1, 后需写入ic4寄存器
                             // sngl为0表示采用级联模式
                             // 设置8085CPU的调用时间间隔(兼容无设)
                             // LTIM为0设置边沿触发方式
    // 往主片中写入ICW2(设置起始中断向量号)
    outb (PIC_M_DATA, 0x20); // 起始中断向量号32, 其余由IRQ接口顺延
    // 往主片中写入ICW3(设置主从级联时用到的引脚)
    outb (PIC_M_DATA, 0x04); // 使用S2引脚连接主片和从片
    // 往主片中写入ICW4
    outb (PIC_M_DATA, 0x01); // 设置uPM位为1(x86处理器)

    // 初始化从片
    // 往从片中写ICW1, 同主片设置
    outb (PIC_S_CTRL, 0x11);
    // 往从片中写ICW2
    outb (PIC_S_DATA, 0x28); // 每个片对应着8个中断向量
    outb (PIC_S_DATA, 0x02); // 从片使用S1引脚连接
    outb (PIC_S_DATA, 0x01); // 设置uPM位为1(x86处理器)

    // 打开主片上IR0, 接受时钟产生的中断
    // 此时向8259A发送的命令操作属于OCW
    outb (PIC_M_DATA, 0xfe);
    outb (PIC_S_DATA, 0xff);

    print_string("pic init done!\n");
}

void idt_init(void) {
    /* idt_init: 初始化中断描述符表
     */
    print_string("idt_init start\n");
    idt_desc_init(); // 初始化中断描述符
    pic_init();      // 初始化中断代理芯片(ex:8259A)
    // 加载idt
    uint64_t idt_operand = (
        (sizeof(idt) - 1) | ((uint64_t)(
            (uint32_t)idt << 16)
        )
    ); // 组装IDTR
    asm volatile (
        "lidt %0"
        :
        : "m" (idt_operand)
    );  // 加载idt
    print_string("idt init done\n");
}
