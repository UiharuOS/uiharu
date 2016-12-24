#ifndef KERNEL_LIB_IO_H
#define KERNEL_LIB_IO_H
#include "stdint.h"

static inline void outb(uint16_t port, uint8_t data) {
    /* outb: 向端口port写入一个字节
     *   port: 指定端口寄存器
     *   data: 待写入的一个字节数据
     *   => inline: 建议编译器将函数编译为内嵌的方式,
     *      就是将所调用函数体的内容在调用处展开, 由call调用变成顺序执行
     *      减少了call函数调用时的上下文保护, 不过体积变大
     *   -> b: QImode (a-d)l; w: HImode (a-d)x
     *   -> a: eax, ax, al ; N:立即数约束(0~255); d edx, dx, dl
     *   => outb %al, %dx
     */
    asm volatile ("outb %b0, %w1"
        :
        : "a" (data), "Nd" (port)
    );
}

static inline void outsw(uint16_t port, const void* addr, uint32_t word_cnt) {
    /* outsw: 将addr处起始的word_cnt个字写入端口port
     *   port: 端口寄存器
     *   addr: 起始内存地址
     *   word_cnt: 待复制的内存大小
     *   => cld ; clean direction, 向上扩展
     *   => rep outsw ; outsw:
     *      Output word from memory location specified in DS:(E)SI
     *      to I/O port specified in DX
     *   -> +: 可读写, 约束的内存(寄存器)->先被读入再被写入
     */
    asm volatile ("cld; rep outsw"
        : "+S" (addr), "+c" (word_cnt)
        : "d" (port)
    );
}

static inline uint8_t inb(uint16_t port) {
    /* inb: 从指定的端口寄存器中读出一个字节的数据
     *   port: 指定的端口寄存器
     *   => inb %dx, $data
     */
    uint8_t data;
    asm volatile ("inb %w1, %b0"
        : "=a" (data)
        : "d" (port)
    );
    return data;
}

static inline void insw(uint16_t port, void* addr, uint32_t word_cnt) {
    /* insw: 从指定的端口读出word_cnt字节到addr起始的内存处
     *   port: 指定的端口寄存器
     *   addr: 空指针void*指向数据缓冲区, 存储读出来的数据
     *   word_cnt: 读出的字节数
     */
    asm volatile ("cld; rep insw"
        : "+D" (addr), "+c" (word_cnt)
        : "d" (port)
        : "memory"
    );
}

#endif
