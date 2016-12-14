#include "bitmap.h"
#include "stdint.h"
#include "string.h"
#include "print.h"
#include "interrupt.h"
#include "debug.h"

void bitmap_init(struct bitmap* btmp) {
    /* bitmap_init: 初始化位图
     *   - 位图上一个函数(模块)构建好, 用此函数初始化
     */
    memset(btmp->bits, 0, btmp->btmp_bytes_len);
}

bool bitmap_scan_test(struct bitmap* btmp, uint32_t bit_idx) {
    /* bitmap_scan_test: 判断bit_idx位是否为1
     *   - 若是: return true
     *   - 否则: return false
     */
    uint32_t byte_idx = bit_idx / 8;  // 向下取整, 数组下标
    uint32_t bit_odd = bit_idx % 8;   // 取余数, 数组内位索引
    // BITMAP_MASK << bit_odd 预先把相应的位置为1与byte_idx进行比较
    return (btmp->bits[byte_idx] & (BITMAP_MASK << bit_odd));
}

int bitmap_scan(struct bitmap* btmp, uint32_t cnt) {
    /* bitmap_scan: 在位图中连续申请cnt个位
     *   - 成功: 返回起始位的位下标(可以找到连续cnt个空闲位)
     *   - 失败: 返回-1
     *   一开始逐字节遍历, 增加跨度, 方便后续查找, 否则退化为位查找
     */
    uint32_t idx_byte = 0;  // 纪录空闲位所在的字节(先定位到字节)
    while (( btmp->bits[idx_byte]==0xff) && (idx_byte < btmp->btmp_bytes_len)) {
        idx_byte++;
    }
    if (idx_byte == btmp->btmp_bytes_len) {
        return -1; // 内存池中已无空闲内存
    }
    ASSERT(idx_byte < btmp->btmp_bytes_len);  // 无空闲内存挂起
    int idx_bit = 0;
    while ((uint8_t)(BITMAP_MASK << idx_bit) & btmp->bits[idx_byte]) {
        // 在位图数组某一字节逐位寻找空闲位(定位到位)
        idx_bit++;
    }
    int bit_idx_start = idx_byte*8 + idx_bit; // 位图中第一个空闲位的[位索引]
    if (cnt == 1) {
        // 如果只要一个空闲位, 返回这个位就行了
        return bit_idx_start;
    }

    uint32_t bit_left = (btmp->btmp_bytes_len*8-bit_idx_start); // 位图中剩余位的个数
    uint32_t next_bit = bit_idx_start + 1;
    uint32_t count = 1;
    bit_idx_start = -1;
    while (bit_left-- > 0) {
        if (!(bitmap_scan_test(btmp, next_bit))) { // next_bit空闲
            count++;
        } else {
            count=0;  // 连续
        }
        if (count == cnt) {
            bit_idx_start = next_bit-cnt+1;  // 起始地址
            break;
        }
        next_bit++;
    }
    return bit_idx_start;
}

void bitmap_set(struct bitmap* btmp, uint32_t bit_idx, int8_t value) {
    /* 将位图btmp的第bit_idx位设为value
     */
    // 位图每一位只能0或1哦!(ps我是不是很可爱)
    ASSERT((value==0) || (value==1));
    uint32_t byte_idx = bit_idx / 8;  // 字节索引
    uint32_t bit_odd = bit_idx % 8;   // 该字节元素的位索引
    if (value) {
        btmp->bits[byte_idx] |=  (BITMAP_MASK << bit_odd);
    } else {
        btmp->bits[byte_idx] &= ~(BITMAP_MASK << bit_odd);
    }
}
