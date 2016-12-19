#ifndef KERNEL_LIB_DLIST_H
#define KERNEL_LIB_DLIST_H
#include "global.h"
#include "boolean.h"

#define offset(struct_type, member) (int)(&((struct_type*)0)->member)
#define elem2entry(struct_type, struct_member_name, elem_ptr) \
        (struct_type*)((int)elem_ptr-offset(struct_type, struct_member_name))

struct list_entry {
    struct list_entry* prev;
    struct list_entry* next;
};

struct list {
    struct list_entry head;
    struct list_entry tail;
};

typedef boolean (function)(struct list_entry*, int arg);

// functions
void list_init(struct list*);
void list_insert_before(struct list_entry* before, struct list_entry* elem);
void list_push(struct list* plist, struct list_entry* elem);
void list_iterate(struct list* plist);
void list_append(struct list* plist, struct list_entry* elem);
void list_remove(struct list* plist, struct list_entry* elem);
struct list_entry* list_pop(struct list* plist);
boolean list_empty(struct list* plist);
uint32_t list_len(struct list* plist);
struct list_entry* list_traversal(struct list* plist, function func, int arg);
boolean elem_find(struct list* plist, struct list_entry* elem);

#endif
