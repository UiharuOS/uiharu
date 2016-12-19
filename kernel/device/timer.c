#include "timer.h"
#include "io.h"
#include "print.h"
#include "stdint.h"
#include "thread.h"
#include "interrupt.h"
#include "debug.h"

#define IRQ0_FREQUENCY     100      // 100HZ/S的CPU时钟中断频率
#define INPUT_FREQUENCY    1193180  // intel 8253定时计数器CLK时钟
#define COUNTER0_VALUE     INPUT_FREQUENCY/IRQ0_FREQUENCY  // 计数器0计数初始值
#define COUNTER0_PORT      0x40     // 8253计数器0寄存器端口
#define COUNTER0_NUM       0        // 8253控制字6、7位: 选择计数器0
#define COUNTER0_MODE      2        // 8253控制字2、3位: 采用比率发生器(Rate Generator)
#define READ_WRITE_LATCH   3        // 8253控制字4、5位: 先读写低字节、后读写高字节
#define PIT_CONTROL_PORT   0x43     // 8253控制字寄存器端口

static void frequency_set(uint8_t counter_port,
                          uint8_t counter_no,
                          uint8_t rwl,
                          uint8_t counter_mode,
                          uint16_t counter_value) {
    /* frequency_set
     *   初始化8253控制字寄存器(8位),
     *   初始化计数器0的初值计数器(16位)
     */
    // 向控制字寄存器中写入控制字
    outb(PIT_CONTROL_PORT, (uint8_t)(
        (counter_no<<6) | (rwl<<4) | (counter_mode<<1))
    );
    // 因为READ_WRITE_LATCH=3(11)
    // 先向计数器0写入初值的低8位
    outb(counter_port, (uint8_t)counter_value);
    // 再写入初值的高8位
    outb(counter_port, (uint8_t)(counter_value >> 8));
}

static void intr_timer_handler(void) {
    struct task_struct* current_thread = get_running_thread_pcb();
    // 通过魔数, 检查内核线程栈是否越界破坏pcb信息
    ASSERT(current_thread->stack_magic == 0x19960411);
    current_thread->all_ticks++;
    ticks++;
    if (current_thread->ticks == 0) {
        schedule();
    } else {
        current_thread->ticks--;
    }
}

void register_handler(uint8_t vector_no, intr_handler function) {
    idt_table[vector_no] = function;
}

void timer_init() {
    /* timer_init
     *  初始化8253寄存器
     */
    print_string("Info)-> intel 8253 timer init start\n");
    frequency_set(COUNTER0_PORT,
                  COUNTER0_NUM,
                  READ_WRITE_LATCH,
                  COUNTER0_MODE,
                  COUNTER0_VALUE
                  /*BCD默认为0, 使用2进制计数, 0代表65536*/);
    print_string("Info)--> register intr_timer_handler done\n");
    register_handler(0x20, intr_timer_handler);
    print_string("Info)-> intel 8253 timer init done\n");
}
