/***
 * memory.h
 * ````````
 *   基于位图的内存池结构
 */
#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include "stdint.h"
#include "bitmap.h"

// 定义属性位用于页表修改
#define PG_P_1   1  // 页表项或页目录项存在的属性位, 存在
#define PG_P_0   0  // 页表项或页目录项存在的属性位, 不存在
#define PG_RW_R  0  // R/W 属性位值, 读/执行
#define PG_RW_W  2  // R/W 属性位值, 读/写/执行
#define PG_US_S  0  // U/S 属性位值, 系统级
#define PG_US_U  4  // U/S 属性位值, 用户级

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

enum pool_flags {
    PF_KERNEL = 1,  // 内核物理内存池
    PF_USER = 2     // 用户物理内存池
};

// for extern
extern struct pool kernel_pool, user_pool;
void mem_init(void);
static void* vaddr_get(enum pool_flags, uint32_t);
uint32_t* pte_ptr(uint32_t);
uint32_t* pde_ptr(uint32_t);
static void* palloc(struct pool*);
static void page_table_add(void*, void*);
void* malloc_page(enum pool_flags, uint32_t);
void* get_kernel_pages(uint32_t);

#endif
