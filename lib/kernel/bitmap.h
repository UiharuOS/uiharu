#ifndef LIB_KERNEL_BITMAP_H
#define LIB_KERNEL_BITMAP_H

#include "global.h"
#include "boolean.h"

/* 用于位图中的&逐位判断*/
#define BITMAP_MASK 1

struct bitmap {
    // 位图
    uint32_t btmp_bytes_len;  /* 位图的字节长度 */
    // 位索引, 指针还是4字节跨度, 需要取余/移位 遍历位
    uint8_t* bits;  /* 一个字节跨度遍历内存 */
};

void bitmap_init(struct bitmap* btmp);
boolean bitmap_scan_test(struct bitmap* btmp, uint32_t bit_idx);
int bitmap_scan(struct bitmap* btmp, uint32_t cnt);
void bitmap_set(struct bitmap* btmp, uint32_t bit_idx, int8_t value);

#endif
