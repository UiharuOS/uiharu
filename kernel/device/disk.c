typedef unsigned int uint32_t;

void read_from_disk(uint32_t sector_no, uint32_t start_sector, uint32_t mem_to) {
    /* read_from_disk: 从硬盘中读取数据到指定内存
     *   - sector_no: 待读取的扇区数
     *   - start_sector: 起始lba扇区号
     *   - mem_to: 指定内存
     */
    asm volatile ("movl %%eax, %%esi\n\t"
                  "movw %%cx, %%di\n\t"
                  "movw $0x1f2, %%dx\n\t"
                  "movb %%cl, %%al\n\t"
                  "outb %%al, %%dx\n\t"
                  "movl %%esi, %%eax\n\t"
                  "movw $0x1f3, %%dx\n\t"
                  "outb %%al, %%dx\n\t"
                  "movb $8, %%cl\n\t"
                  "shr %%cl, %%eax\n\t"
                  "movw $0x1f4, %%dx\n\t"
                  "outb %%al, %%dx\n\t"
                  "shr %%cl, %%eax\n\t"
                  "movw $0x1f5, %%dx\n\t"
                  "outb %%al, %%dx\n\t"
                  "shr %%cl, %%eax\n\t"
                  "or $0xe0, %%al\n\t"
                  "movw $0x1f6, %%dx\n\t"
                  "outb %%al, %%dx\n\t"
                  "movw $0x1f7, %%dx\n\t"
                  "movb $0x20, %%al\n\t"
                  "outb %%al, %%dx\n\t"
                  ".not_ready:\n\t"
                  "nop\n\t"
                  "in %%dx, %%al\n\t"
                  "and $0x88, %%al\n\t"
                  "cmp $0x88, %%al\n\t"
                  "jnz .not_ready\n\t"
                  "movw %%di, %%ax\n\t"
                  "movw $256, %%dx\n\t"
                  "mul %%dx\n\t"
                  "movw %%ax, %%cx\n\t"
                  "movw $0x1f0, %%dx\n\t"
                  ".go_on_read:\n\t"
                  "in %%dx, %%ax\n\t"
                  "movw %%ax, (%%bx)\n\t"
                  "add $2, %%bx\n\t"
                  "loop .go_on_read\n\t"
                  "ret\n\t"
                 : /* no outputs */
                 : "c" (sector_no), "a" (start_sector), "b" (mem_to)
                 // : "eax", "ebx", "ecx"
                 );
}
