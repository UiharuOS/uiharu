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

char* intr_name[IDT_DESC_CNT];         // 保存异常[名字](你的名字把uiharu写完看)
intr_handler idt_table[IDT_DESC_CNT];  // 中断处理程序数组, 每个中断向量索引一个中断处理程序
void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler function);
// static 函数声明
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

static void general_intr_handler(uint8_t vec_nr) {
    /* general_intr_handler: 中断处理函数, 出现异常时处理
     *   vec_tr: 中断向量号
     *   相关中断如果没有定义相应的中断处理函数, 则调用此通用中断处理函数(初始化)
     */
    if(vec_nr == 0x27 || vec_nr == 0x2f) {
        // IRQ7 和 IRQ15 会产生伪中断, 无需处理
        // 伪中断(spurious interrupt) 
        return;
    }
    print_string("int vector : 0x");
    print_int(vec_nr);
    print_string("\n");
}

static void exception_init(void) {
    /* exception_init: 完成一般中断处理函数注册及异常名称注册
     */
    int i;
    for(i=0; i<IDT_DESC_CNT; i++) {
        // 初始化中断程序数组; 初始化中断异常数组
        idt_table[i] = general_intr_handler;  // 中断处理程序
        intr_name[i] = "unknown";  // 先统一初始化为 unknown, 保证33个不为空
    }
    // 初始化20个中断异常
    intr_name[0] = "#DE Divide Error";
    intr_name[1] = "#DB Debug Exception";
    intr_name[2] = "NMI Interrupt";
    intr_name[3] = "#BP Breakpoint Exception";
    intr_name[4] = "#OF Overflow Exception";
    intr_name[5] = "#BR BOUND Range Exceeded Exception";
    intr_name[6] = "#UD Invalid Opcode Exception";
    intr_name[7] = "#NM Device Not Available Exception";
    intr_name[8] = "#DF Double Fault Exception";
    intr_name[9] = "Coprocessor Segment Overrun";
    intr_name[10]= "#TS Invalid TSS Exception";
    intr_name[11]= "#NP Segment Not Present";
    intr_name[12]= "#SS Stack Fault Exception";
    intr_name[13]= "#GP General Protection Exception";
    intr_name[14]= "#PF Page-Fault Exception";
    // intr_name[15]= "第15项是intel保留项, 未使用"
    intr_name[16]= "#MF x87 FPU Floating-Point Error";
    intr_name[17]= "#AC Alignment Check Exception";
    intr_name[18]= "#MC Machine-check Exception";
    intr_name[19]= "#XF SIMD Floating-Point Exception";
}

static void pic_init(void) {
    /* pic_init: 初始化8259A PIC芯片
     *   向8259A主片、从片要求的端口发送设置位,
     *   即可进行初始化(ICW寄存器组)和操作(OCW寄存器组)
     *   ICW: 设置是否级联、起始中断向量号(顺延), 中断结束模式
     *      - ICW1: 设置连接方式和中断信号的触发方式(电平触发、边沿触发)
     *      - ICW2: 设置起始中断向量号, 硬件IRQ0映射到的中断向量号, 其余顺延
     *      - ICW3: (需要写入主片的0x21从片的0xA1端口) 1->连接从片 0->连接外设
     *              低3位指定与主片级联的IRQ端口号
     *      - ICW4: 设置8259A的工作模式(写入主片的0x21及从片的0xA1端口)
     *   OCW: 操作8259A芯片
     *      - OCW1: 屏蔽连接在8259A上的外设中断信号, OCW1写入了IMR寄存器
     *              (需要写入主片的0x21端口,从片的0xA1端口)
     *              每一位对应IRQ接口, 体现了位图的思想(方便定位与控制中断)
     *      - OCW2: 设置中断结束方式以及优先级模式
     *              (需要写入主片的0x20以及从片的0xA0端口)
     *      - OCW3: 设定特殊屏蔽方式和查询方式
     *              (需要写入主片的0x20以及从片的0xA0端口)
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
    idt_desc_init();  // 初始化中断描述符
    exception_init(); // 异常名初始化并注册通常的中断处理函数
    pic_init();       // 初始化中断代理芯片(ex:8259A)
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
