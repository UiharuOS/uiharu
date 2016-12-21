#ifndef LIB_STRING_H
#define LIB_STRING_H

#include "stdint.h"
#include "type.h"

void memset(void* dst, uint8_t value, uint32_t size);
void memcpy(void* dst, const void* src, uint32_t size);
int memcmp(const void* a, const void* b, uint32_t size);

int8_t* strcpy(int8_t* dst, const int8_t* src);
uint32_t strlen(const int8_t* str);
int8_t strcmp(const int8_t* a, const int8_t* b);
int8_t* strchr(const int8_t* str, const uint8_t ch);
int8_t* strrchr(const int8_t* str, const uint8_t ch);
int8_t* strcat(int8_t* dst, const int8_t* src);
uint32_t strchrs(const int8_t* str, uint8_t ch);

#endif
