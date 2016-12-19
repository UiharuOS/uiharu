/***
 * memory.c
 * ````````
 *   内存池(虚拟、物理内存地址)分配管理
 */
#include "memory.h"
#include "stdint.h"
#include "print.h"
#include "bitmap.h"
#include "global.h"
#include "debug.h"
#include "string.h"

/* 每页规划为4KB */
#define PG_SIZE 4096
/* 位图地址 */
#define MEM_BITMAP_BASE 0xc009a000   // 因为0xc009f000是内核主线程的栈顶<loader.S:enter_kernel>
                                     // 0xc009e000是内核主线程main的pcb(0xc009e000~0xc009efff)起始地址
                                     // 一个页框大小的位图可以管理(4*1024*4*1024*8)128MB的内存
                                     // 把位图放在0xc009a000
                                     // 最大可管理(0x4000/1024)16KB也就是4个页框,512MB的内存
/* 堆内存起始地址
 *   动态内存分配从堆中拿, kernel_pool和user_pool管理的内存就相当于堆内存
 */
#define K_HEAP_START    0xc0100000   // 虚拟地址空间起始1MB给了内核和loader, mbr(内核加载后会被覆盖)
                                     // 堆内存安排在1MB以上, 而且从0xc0100000开始可以让虚拟地址在逻辑上连续
                                     // 不过需要绕过第0项和第768项, 这两个页目录项用作虚拟地址与低端1MB的映射
#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)  // 返回虚拟地址的高10位->页目录项索引
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)  // 返回虚拟地址的中间10位->页表项索引

struct pool kernel_pool, user_pool;  // 内核物理内存池和用户物理内存池
struct virtual_addr kernel_vaddr;    // 给内核分配虚拟地址的虚拟地址池

static void mem_pool_init(uint32_t all_mem) {
    /* mem_pool_init 初始化物理内存池
     *   all_mem: 机器物理内存总容量大小
     *   总容量已经在loader中获取了, 存储在total_mem_bytes(32位大小地址0xb00)中
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
    uint32_t kernel_bitmap_length = kernel_free_pages / 8; // 字节长度(一位管理1页, 共kernel_free_pages位)
    uint32_t user_bitmap_length = user_free_pages / 8;
    uint32_t kernel_phyaddr_start = used_mem;  // 内核物理内存池起始地址设为used_mem->256个分配页表之上
    uint32_t user_phyaddr_start = kernel_phyaddr_start + kernel_free_pages*PG_SIZE; // free_pages也符合used_mem的设置

    /* 初始化内核物理内存池 */
    kernel_pool.phy_addr_start = kernel_phyaddr_start;
    kernel_pool.pool_size = kernel_free_pages*PG_SIZE;
    kernel_pool.pool_bitmap.btmp_bytes_len = kernel_bitmap_length;
    kernel_pool.pool_bitmap.bits = (void*)MEM_BITMAP_BASE;

    /* 初始化用户物理内存池 */
    user_pool.phy_addr_start = user_phyaddr_start;
    user_pool.pool_size = user_free_pages*PG_SIZE;
    user_pool.pool_bitmap.btmp_bytes_len = user_bitmap_length;
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

static void* vaddr_get(enum pool_flags pf, uint32_t pg_cnt) {
    /* vaddr_get: 在pf表示的物理内存池对应的虚拟内存池中申请pg_cnt个虚拟页
     * - pf: 内存池标识: 用户/内核
     * - pg_cnt: 申请虚拟页的页数
     *   申请成功返回虚拟页的起始地址, 失败则返回NULL(空指针)
     */
    int32_t vaddr_start = 0, bit_idx_start = -1;
    uint32_t cnt = 0;
    if (pf == PF_KERNEL) {
        // bit_idx_start 位图中连续分配pg_cnt个页的起始位的位下标
        bit_idx_start = bitmap_scan(&kernel_vaddr.vaddr_bitmap, pg_cnt);
        if (bit_idx_start == -1) {
            return NULL;  // 无连续空闲内存可以分配
        }
        while (cnt < pg_cnt) {
            bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx_start+cnt++, 1); // 全部置为1,即已经分配出去了
        }
        // 将bit_idx_start转换为虚拟地址
        vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start*PG_SIZE;
    } else {
        /* 此处留给学园都市(用户空间) */
    }
    return (void*)vaddr_start;
}

uint32_t* pte_ptr(uint32_t vaddr) {
    /* pte_ptr: 得到虚拟地址vaddr对应的物理页起始物理地址 */
    uint32_t* pte = (uint32_t*)(0xffc00000 + \
                    ((vaddr & 0xffc00000) >> 10) + \
                    PTE_IDX(vaddr) * 4);
    return pte; // pte是指向虚拟地址
                // -- (高10位全是1, 中间10位vaddr高10位, 低12位是vaddr中间10位左移2位)
                // 的指针
}

uint32_t* pde_ptr(uint32_t vaddr) {
    /* pde_ptr: 得到虚拟地址vaddr对应的pde指针 */
    uint32_t* pde = (uint32_t*)((0xfffff000)+PDE_IDX(vaddr)*4);
    return pde; // pde是指向虚拟地址
                // -- (高20位全是1, 后12位是vaddr高10位左移2位)
                // 的指针
}

static void* palloc(struct pool* m_pool) {
    /* palloc: 在m_pool指向的物理内存池中分配1个物理页
     * 成功则返回页框的物理地址, 失败则返回NULL
     */
    uint32_t bit_idx = bitmap_scan(&m_pool->pool_bitmap, 1);
    if (bit_idx == -1) {
        return NULL;
    }
    bitmap_set(&m_pool->pool_bitmap, bit_idx, 1);
    uint32_t page_phyaddr = ((bit_idx*PG_SIZE)+m_pool->phy_addr_start);
    return (void*)page_phyaddr;
}

static void page_table_add(void* vaddr, void* page_phyaddr) {
    /* page_table_add: 添加虚拟地址与物理页地址的映射 */
    uint32_t _vaddr = (uint32_t)vaddr;  // 把指向任意类型内存区域的指针转化为地址
    uint32_t _page_phyaddr = (uint32_t)page_phyaddr;
    uint32_t* pde = pde_ptr(_vaddr);
    uint32_t* pte = pte_ptr(_vaddr);

    if (*pde & 0x00000001) { // 该虚拟地址查找到的页目录项存在(页表存在)
        if (!(*pte & 0x00000001)) {
            // 该虚拟地址查找到的页表项不存在(物理页不存在, 还没有被分配)
            // pte中存放的是物理页的起始物理地址, 与物理地址参数建立映射
            *pte = (_page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        } else { // 该物理页已经被分配了
            DEBUGGER("the page pte point to, has been assigned!");
            // *pte = (_page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);  // 覆盖
        }
    } else { // 页表不存在, 虚拟地址索引范围超过了loader创建的页表范围, 创建新的页表
        // 页表属于系统资源, 页表页框空间从内核内存池中拿
        uint32_t pde_phyaddr = (uint32_t)palloc(&kernel_pool); // pde_phyaddr存储的是申请1页的物理起始地址
        *pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        // vaddr的pte低12位清零, 得到vaddr对应物理页的起始地址(pte中存放的是高20位)
        // 保证新分配的物理页干净
        memset((void*)((int)pte & 0xfffff000), 0, PG_SIZE);
        ASSERT(!(*pte & 0x00000001)); // 此时pte应该还不存在
        *pte = (_page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
    }
}

void* malloc_page(enum pool_flags pf, uint32_t pg_cnt) {
    /* malloc_page:
     * - 通过vaddr_get在虚拟内存池中申请虚拟地址
     * - 通过palloc在物理内存池中申请1个物理页
     * - 通过page_table_add将虚拟地址和物理页地址在页表中完成映射
     */
    void* vaddr_start = vaddr_get(pf, pg_cnt);
    if (vaddr_start == NULL) {
        return NULL;
    }
    uint32_t vaddr = (uint32_t)vaddr_start;
    uint32_t cnt = pg_cnt;
    struct pool* mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;
    while (cnt-- > 0) {
        void* page_phyaddr = palloc(mem_pool);
        if (page_phyaddr == NULL) {
            return NULL;
        }
        page_table_add((void*)vaddr, page_phyaddr);  // 逐项添加虚拟地址与分配物理页的映射
        vaddr += PG_SIZE;
    }
    return vaddr_start;  // 返回虚拟地址, 程序中使用的是逻辑上的虚拟地址
}

void* get_kernel_pages(uint32_t pg_cnt) {
    /* get_kernel_pages: 从内核物理内存池中申请页内存
     */
    void* vaddr = malloc_page(PF_KERNEL, pg_cnt);
    if (vaddr != NULL) {
        memset(vaddr, 0, pg_cnt*PG_SIZE);
    }
    return vaddr;
}
