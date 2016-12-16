/***
 * memory.c
 * ````````
 *   内存池(虚拟、物理内存地址)分配管理
 */
#include "memory.h"
#include "stdint.h"
#include "print.h"

/* 每页规划为4KB */
#define PG_SIZE 4096
/* 位图地址 */
#define MEM_BITMAP_BASE 0xc009a000   // 因为0xc009f000是内核主线程的栈顶<loader.S:enter_kernel>
                                     // 0xc009e000是内核主线程的pcb(0xc009e000~0xc009efff)
                                     // 一个页框大小的位图可以管理(4*1024*4*1024*8)128MB的内存
                                     // 把位图放在0xc009a000
                                     // 最大可管理(0x4000/1024)16KB也就是4个页框,512MB的内存
/* 堆内存起始地址 */
#define K_HEAP_START    0xc0100000   // 虚拟地址空间起始1MB给了内核kernel.bin和kernel image
                                     // 堆内存安排在1MB以上, 而且从0xc0100000开始可以让虚拟地址在逻辑上连续

struct pool kernel_pool, user_pool;  // 内核物理内存池和用户物理内存池
struct virtual_addr kernel_vaddr;    // 给内核分配虚拟地址的虚拟地址池

static void mem_pool_init(uint32_t all_mem) {
    /* mem_pool_init 初始化物理内存池
     *   all_mem: 机器物理内存总容量大小
     *   总容量已经在loader中获取了, 存储在total_mem_bytes(32位0xb00)中
     */
    print_string("Info)--> mem_pool_init start\n");
    // 总共初始化了256个页表
    // 分布在物理内存低端1MB以上, "第1个"页表是页目录表(4KB, 第1023个页目录项指向于此)
    uint32_t page_table_size = PG_SIZE * 256;
    uint32_t used_mem = page_table_size + 0x100000;  // 加上低端1MB内存
    uint32_t free_mem = all_mem - used_mem;          // 空闲内存的字节大小(池管理的实际上就是空闲内存)
    uint16_t all_free_pages = free_mem / PG_SIZE;    // 忽略余数, 不足一页的内存不考虑分配
    uint16_t kernel_free_pages = all_free_pages / 2; // 物理内存空间采用对半分的策略
    uint16_t user_free_pages = all_free_pages - kernel_free_pages;
    uint32_t kernel_bitmap_length = kernel_free_pages / 8; // 字节长度
    uint32_t user_bitmap_length = user_free_pages / 8;  
    uint32_t kernel_phyaddr_start = used_mem;  // ?
    uint32_t user_phyaddr_start = kernel_phyaddr_start + kernel_free_pages*PG_SIZE;

    /* 初始化内核物理内存池 */
    kernel_pool.phy_addr_start = kernel_phyaddr_start;
    kernel_pool.pool_size = kernel_free_pages*PG_SIZE;
    kernel_pool.pool_bitmap.btmp_bytes_len = kernel_bitmap_length;
    /* 初始化用户物理内存池 */
    user_pool.phy_addr_start = user_phyaddr_start;
    user_pool.pool_size = user_free_pages*PG_SIZE;
    user_pool.pool_bitmap.btmp_bytes_len = user_bitmap_length;

    /* 初始化内核物理内存池位图指针 */
    kernel_pool.pool_bitmap.bits = (void*)MEM_BITMAP_BASE;
    /* 初始化用户物理内存池位图指针 */
    user_pool.pool_bitmap.bits =(void*)(MEM_BITMAP_BASE + kernel_bitmap_length);

    /* 输出内存池信息 */
    print_string("Info)----> kernel_pool_bitmap_start:");
    print_int((int)kernel_pool.pool_bitmap.bits, 'H');
    print_string("\nInfo)----> kernel_pool_phyaddr_start: ");
    print_int(kernel_pool.phy_addr_start, 'H');
    print_string("\nInfo)----> user_pool_bitmap_start: ");
    print_int((int)user_pool.pool_bitmap.bits, 'H');
    print_string("\nInfo)----> user_pool_phyaddr_start: ");
    print_int(user_pool.phy_addr_start, 'H');
    print_string("\n");

    /* 初始化位图:将位图置0 */
    bitmap_init(&kernel_pool.pool_bitmap);
    bitmap_init(&user_pool.pool_bitmap);

    /* 初始化内核虚拟地址位图,按实际物理内存大小生成数组 */
    kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kernel_bitmap_length;

    kernel_vaddr.vaddr_bitmap.bits = (void*)(MEM_BITMAP_BASE + kernel_bitmap_length + user_bitmap_length);

    kernel_vaddr.vaddr_start = K_HEAP_START;
    bitmap_init(&kernel_vaddr.vaddr_bitmap);
    print_string("Info)--> mem_pool_init done\n");

}

void mem_init() {
    /* mem_init: 内存管理系统初始化 */
    print_string("Info)-> mem_init start\n");
    // 0xb00是loader统计机器物理内存(bios:0x15:e820)结果变量的起始存储位置
    // 该变量定义dd: 32字节.
    // 所以把该内存转换为指向相应内存大小的指针再用*取值就可以拿到该变量的值
    uint32_t mem_bytes_total = (*(uint32_t*)(0xb00)); // amazing!
    mem_pool_init(mem_bytes_total);
    print_string("Info)-> mem_init done\n");
}
