/***
 * inline assembly:
 * --> its just some assembly routines written as inline functions
 * --> "constraint" (C expression) <- Operands
 */
#include "stdint.h"

void read_from_disk(uint32_t sector_no, uint32_t start_sector, uint32_t mem_to) {
    /* read_from_disk: 从硬盘中读取数据到指定内存
     *   - sector_no: 待读取的扇区数
     *   - start_sector: 起始lba扇区号
     *   - mem_to: 指定内存
     */
    asm volatile ("movl %eax, %esi"
                  "movw %cx, %di"
                  "movw $0x1f2, %dx"
                  "movw %cl, %al"
                  "outw %al, %dx"
                  "movl %esi, %eax"
                  "movw $0x1f3, %dx"
                  "outw %al, %dx"
                  "movw $8, %cl"
                  "shr %cl, %eax"
                  "movw $0x1f4, %dx"
                  "outw %al, %dx"
                  "shr %cl, %eax"
                  "movw $0x1f5, %dx"
                  "outw %al, %dx"
                  "shr %cl, %eax"
                  "or $0xe0, %al"
                  "movw $0x1f6, %dx"
                  "outw %al, %dx"
                  "movw $0x1f7, %dx"
                  "movw $0x20, %al"
                  "outw %al, %dx"
                  ".not_ready:"
                  "nop"
                  "in %dx, %al"
                  "and $0x88, %al"
                  "cmp $0x88, %al"
                  "jnz .not_ready"
                  "movw %di, %ax"
                  "movw $256, %dx"
                  "mul %dx"
                  "movw %ax, %cx"
                  "movw $0x1f0, %dx"
                  ".go_on_read:"
                  "in %dx, %ax"
                  "movw %ax, (%bx)"
                  "add $2, %bx"
                  "loop .go_on_read"
                 : /* no outputs */
                 : "c"(sector_no), "a"(start_sector), "b"(mem_to)
                 : "eax", "ebx", "ecx"
                 );
}
