#include "global.h"
#include "print.h"
#include "thread.h"
#include "string.h"

#define PG_SIZE 4096

/* Task State Segment结构
 * -- TSS是硬件(inter)要求的,程序员提供
 * -- CPU使用TSS做任务切换
 */
struct tss {
    uint32_t backlink;
    uint32_t* esp0;
    uint32_t ss0;
    uint32_t* esp1;
    uint32_t ss1;
    uint32_t* esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t (*eip) (void);
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint32_t trace;
    uint32_t io_base;
};

static struct tss tss;  // 交给CPU用的tss

void update_tss_esp(struct task_struct* thread) {
    /* 更新tss的esp0属性为thread的0级栈
     * intel要求每个任务一个TSS, 然后切换不同的任务重载相应的TSS.
     * 但这样既耗存储空间, 同时TSS是在内存中的, 每次重载需要大量时间.
     * 这个方法并不高效.
     * 所以这里在每次任务切换时, 只更新提供给CPU的TSS的0级栈指针为相应任务PCB的
     * 0级线程栈. 这个TSS是不得不给CPU的,因为CPU会依据TSS做任务切换.
     */
    tss.esp0 = (uint32_t*)((uint32_t)thread + PG_SIZE);
}

static struct gdt_desc make_gdt_desc( \
        uint32_t* desc_addr,    \
        uint32_t limit,         \
        uint8_t attr_low,       \
        uint8_t attr_high
) {
    /* 构造gdt描述符 */
    uint32_t desc_base = (uint32_t)desc_addr;
    struct gdt_desc desc;
    desc.limit_low_word = limit & 0x0000ffff;
    desc.base_low_word = desc_base & 0x0000ffff;
    desc.base_mid_byte = ((desc_base & 0x00ff0000) >> 16);
    desc.attr_low_byte = (uint8_t)(attr_low);
    desc.limit_high_attr_high = (((limit & 0x000f0000) >> 16) + (uint8_t)(attr_high));
    desc.base_high_byte = desc_base >> 24;
    return desc;
}

void tss_init() {
    /* 在gdt中创建tss并加载gdt */
    print_string("Info)-> tss_init start\n");
    uint32_t tss_size = sizeof(tss);
    memset(&tss, 0, tss_size); // 将tss描述符表所在的内存初始化清0
    tss.ss0 = SELECTOR_K_STACK;
    tss.io_base = tss_size;
    *((struct gdt_desc*)0xc0000920) = make_gdt_desc(
            (uint32_t*)&tss,
            tss_size - 1,
            TSS_ATTR_LOW,
            TSS_ATTR_HIGH);
    *((struct gdt_desc*)0xc0000928) = make_gdt_desc(
            (uint32_t*)0,
            0xfffff,
            GDT_DATA_ATTR_LOW_DPL3,
            GDT_ATTR_HIGH);
    // lgdt: 16为表界限&32位表的起始地址
    // 8*7(个描述符)-1 | 64位32位地址
    uint64_t gdt_operand = ((8*7-1) | ((uint64_t)(uint32_t)0xc0000900 << 16));
    asm volatile (
        "lgdt %0"
        : /* no outputs */
        : "m" (gdt_operand)
    );
    asm volatile (
        "ltr %w0"
        : /* no outputs */
        : "r" (SELECTOR_TSS)
    );
    print_string("Info)-> tss_init done\n");
}
