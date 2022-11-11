#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "klib.h"
#include "debug.h"

void append_list_tail(struct head_list_t *list, struct list_t *item)
{
    item->next = NULL;

    if (is_list_empty(list)) {
        list->next = item;
        list->tail = item;
    }
    else {
        list->tail->next = item;
        list->tail = item;
    }
}

struct list_t* remove_list_head(struct head_list_t *list)
{
    struct list_t *item;

    if (is_list_empty(list)) {
        return NULL;
    }

    item = list->next;
    list->next = item->next;
    
    if (list->next == NULL) {
        list->tail = NULL;
    }
    
    return item;
}

bool is_list_empty(struct head_list_t *list)
{
    return (list->next == NULL);
}