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
    struct bitmap vaddr_bitmap;  // 虚拟地址位图
    uint32_t vaddr_start;        // 虚拟地址起始地址
};

/* 内存池 */

extern struct pool kernel_pool, user_pool;
void mem_init(void);

#endif
