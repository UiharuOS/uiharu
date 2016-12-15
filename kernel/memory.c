#include "memory.h"
#include "stdint.h"
#include "print.h"

/* 每页规划为4KB */
#define PG_SIZE 4096
/* 位图地址 */
#define MEM_BITMAP_BASE 0xc009a000
/* 堆内存起始地址 */
#define K_HEAP_START    0xc0100000 

/* 内存池结构 */
struct pool {
    struct bitmap pool_bitmap;  // 内存池位图结构, 管理内存
    uint32_t phy_addr_start;    // 内存池管理的物理内存的起始地址
    uint32_t pool_size;         // 内存池字节容量
};

struct pool kernel_pool, user_pool;  // 内核内存池和用户内存池
struct virtual_addr kernel_vaddr;    // 给内核分配虚拟地址

static void mem_pool_init(uint32_t all_mem) {
    /* mem_pool_init 初始化内存池
     *   all_mem: 内存池的总大小
     */
    print_string("mem_pool_init start\n");
    // 页表大小: 1页的页目录表 + 第0项和第768项指向的同一个页表
    //           + 第769~1022个页目录项指向的254个页表, 共256个页框
    uint32_t page_table_size = PG_SIZE * 256;
    uint32_t used_mem = page_table_size + 0x100000; // 加上低端1MB内存
    uint32_t free_mem = all_mem - used_mem;
    uint16_t all_free_pages = free_mem / PG_SIZE; // 不足一页的内存不考虑分配
}
