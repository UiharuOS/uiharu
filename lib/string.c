#include "string.h"
#include "global.h"
#include "debug.h"

void memset(void* dst, uint8_t value, uint32_t size) {
    /* memset: 将dst起始的size字节的内存每一字节都置为value
     *   - 用于连续内存的初始化, 比如清0
     *   - 比汇编做清0, 不知道方便到哪里去了😄
     */
    ASSERT(dst != NULL & size != 0);
    uint8_t* _dst = (uint8_t*)dst;
    while (size-- > 0) {
        *_dst++ = value;
    }
}

void memcpy(void* dst, const void* src, uint32_t size) {
    /* memcpy: 拷贝内存src起始处size字节数据到内存dst
     *   - 以4字节为单位拷贝
     */
    ASSERT(dst != NULL && src != NULL);
    uint8_t* _dst = (uint8_t*)dst;
    const uint8_t* _src = (const uint8_t*)src;
    while (size-- > 0) {
        *_dst++ = *_src++;
    }
}

int memcmp(const void* a, const void* b, uint32_t size) {
    /* memcmp: 比较a, b起始处size字节的数据是否相等
     *   - 若相等        : 返回0
     *   - 若asize>bsize : 返回1
     *   - 若asize<bsize : 返回-1
     */
    const int8_t* _a = a;
    const int8_t* _b = b;
    ASSERT(_a != NULL && _b != NULL);
    while (size-- > 0) {
        if (*_a != *_b) {
            return *_a > *_b ? 1 : -1;
        }
        _a++; _b++;
    }
    return 0;
}

int8_t* strcpy(int8_t* dst, const int8_t* src) {
    /* strcpy: 将字符串从src复制到dst
     */
    ASSERT(dst != NULL && src != NULL);
    uint8_t* _dstart = dst;     // _dstart保存目的字符串的起始地址
    while ((*dst)&&(*dst++ = *src++)&&(*src));
    return _dstart;
}

uint32_t strlen(const int8_t* str) {
    /* strlen: 返回字符串长度(不包含'\0'[NULL])
     */
    ASSERT(str != NULL);
    uint32_t len = 0;
    const int8_t* _sstart = str;
    while (*++_sstart) { len++; }
    return len;
}

int8_t strcmp(const int8_t* a, const int8_t* b) {
    /* strcmp: 比较两个字符串a, b是否相等
     *   - 若a > b: return 1
     *   - 否则: return -1
     */
    ASSERT(a != NULL && b != NULL);
    while (*a != 0 && *a == *b) {
        a++; b++;
    }
    // 这里刷了一个小技巧
    // *a > *b 成立 return 1
    // 否则就是 *a == *b, 正好表达式为false, return 0
    // 少写一点代码...
    return *a < *b ? -1 : *a > *b;
}

int8_t* strchr(const int8_t* str, const uint8_t ch) {
    /* strchr: 从左到右查找字符串str中第一次出现ch字符的地址
     */
    ASSERT(str != NULL);
    while (*++str) {
        if (*str == ch) {
            return (int8_t*)str;  // 强制类型转换const -> char*
        }
    }
    return NULL;
}

int8_t* strrchr(const int8_t* str, const uint8_t ch) {
    /* strrchr: 从右到左查找字符串str中最后一次出现ch字符的地址
     */
    ASSERT(str != NULL);
    const int8_t* last_char = NULL;
    while (*++str) {
        if (*str == ch) {
            last_char = str;
        }
    }
    return (int8_t*)last_char;  // last_char初值为NULL
}

int8_t* strcat(int8_t* dst, const int8_t* src) {
    /* strcat: 将字符串src拼接到dst后, 返回拼接后的串地址
     */
    ASSERT(dst != NULL && src != NULL);
    int8_t* _dend = dst;
    while (*++_dend); // dst最后
    --_dend; // 去掉最后一位 '\0'
    while ((*_dend++ == *src++));
    return dst;
}

uint32_t strchrs(const int8_t* str, uint8_t ch) {
    /* strchrs: 在字符串str中查找字符ch出现的次数
     */
    ASSERT(str != NULL);
    uint32_t ch_cnt = 0;
    const uint8_t* _str = str;
    while (*++_str) {
        if (*_str == ch) {
            ch_cnt++;
        }
    }
    return ch_cnt;
}
