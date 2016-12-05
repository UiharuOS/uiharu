#ifndef _LIB_IO_H
#define _LIB_IO_H
#include "stdint.h"

static inline void outb(uint16_t port, uint8_t data) {
    /* outb: 向端口port写入一个字节
     *   port: 指定端口寄存器
     *   data: 待写入的一个字节数据
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
     */
    asm volatile ("cld; rep outsw"
        : "+S" (addr), "+c" (word_cnt)
        : "d" (port)
    );
}

static inline uint8_t inb(uint16_t port) {
    /* inb: 从指定的端口寄存器中读出一个字节的数据
     *   port: 指定的端口寄存器
     */
    uint8_t data;
    asm volatile ("inb %w1, %b0"
        : "=a" (data)
        : "Nd" (port)
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
