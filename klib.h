#ifndef _KLIB_H
#define _KLIB_H

#include <stdint.h>
#include <stdbool.h>

struct list_t {
    struct list_t *next;
};

struct head_list_t {
    struct list_t *next;
    struct list_t *tail;
};

// Linked list functions
void append_list_tail(struct head_list_t *list, struct list_t *item);
struct list_t *remove_list_head(struct head_list_t *list);
struct list_t* remove_list(struct head_list_t *list, int wait);
bool is_list_empty(struct head_list_t *list);

// Memory load/store/compare primitives.
void memset(void *buff, char c, int size);
void memmove(void *dst, void *src, int size);
void memcpy(void *dst, void *src, int size);
int memcmp(void *s1, void *s2, int size);

#endif
