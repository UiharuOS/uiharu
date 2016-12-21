#include "dlist.h"
#include "interrupt.h"
#include "stdint.h"

void list_init(struct list* list) {
    list->head.prev = 0;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
    list->tail.next = 0;
}

void list_insert_before(struct list_entry* before, struct list_entry* elem) {
    enum intr_status old_status = intr_disable();  // close interrupt
    before->prev->next = elem;
    elem->prev = before->prev;
    elem->next = before;
    before->prev = elem;
    intr_set_status(old_status);  // resume interrupt
}

void list_push(struct list* plist, struct list_entry* elem) {
    list_insert_before(plist->head.next, elem);
};

void list_append(struct list* plist, struct list_entry* elem) {
    list_insert_before(&plist->tail, elem);
};

void list_remove(struct list* plist, struct list_entry* elem) {
    enum intr_status old_status = intr_disable();  // close interrupt
    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;
    intr_set_status(old_status);  // resume interrupt
}

struct list_entry* list_pop(struct list* plist) {
    struct list_entry* _elem = plist->head.next;
    list_remove(plist, _elem);
    return _elem ;
};

boolean elem_find(struct list* plist, struct list_entry* elem) {
    struct list_entry* _elem = plist->head.next;
    while (_elem != &plist->tail) {
        if (_elem == elem) {
            return true;
        }
        elem = elem->next;
    }
    return false;
}

struct list_entry* list_traversal(struct list* plist, function func, int arg) {
    struct list_entry* elem = plist->head.next;
    if (list_empty(plist)) {
        return 0;
    }
    while (elem != &plist->tail) {
        if (func(elem, arg)) {
            return elem;
        }
        elem = elem->next;
    }
    return 0;
};

uint32_t list_len(struct list* plist) {
    uint32_t count = 0;
    struct list_entry* _elem = plist->head.next;
    while (_elem != &plist->tail) {
        count++;
        _elem = _elem->next;
    }
    return count;
}

boolean list_empty(struct list* plist) {
    return (plist->head.next == &plist->tail ? true : false);
}
