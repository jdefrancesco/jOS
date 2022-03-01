#ifndef _KLIB_H
#define _KLIB_H

#include <stdint.h>
#include <stddef.h>

void memset(void *buff, uint8_t c, size_t len);
void memmove(void *dst, void *src, size_t len);
void memcpy(void *dst, void *src, size_t len);
int memcmp(const void *s1, const void *s2, size_t n);

#endif
