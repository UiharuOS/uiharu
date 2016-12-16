/***
 * memory.h
 * ````````
 *   基于位图的内存池结构
 */
#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include "stdint.h"
#include "bitmap.h"

/* 虚拟内存地址池 */
struct virtual_addr {
    struct bitmap vaddr_bitmap;  // 虚拟地址池位图
    uint32_t vaddr_start;        // 虚拟地址起始地址
    // 虚拟地址空间相对于物理地址空间是逻辑上的, 是无限的
    // 所以没有定义pool_size
};

/* 物理内存地址池 */
struct pool {
    struct bitmap pool_bitmap;   // 物理地址池位图
    uint32_t phy_addr_start;     // 物理地址起始地址
    uint32_t pool_size;          // 该内存池的字节容量
};

// for extern
extern struct pool kernel_pool, user_pool;
void mem_init(void);

#endif
