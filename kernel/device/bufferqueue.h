#ifndef KERNEL_DEVICE_BUFFERQUEUE
#define KERNEL_DEVICE_BUFFERQUEUE

#include "stdint.h"
#include "thread.h"
#include "sync.h"
#include "type.h"

#define buffer_size 64

struct bufferqueue {
    struct lock buffer_lock;
    char buffer[buffer_size];
    int32_t head;
    int32_t tail;
    struct task_struct* current_consumer;
    struct task_struct* current_producer;
};

void bufferqueue_init(struct bufferqueue*);
int32_t next_pos(int32_t);
boolean bufferqueue_full(struct bufferqueue*);
boolean bufferqueue_empty(struct bufferqueue*);
static void bufferqueue_sleep(struct task_struct**);
static void bufferqueue_awake(struct task_struct**);
char bufferqueue_getchar(struct bufferqueue*);
void bufferqueue_putchar(struct bufferqueue*, char);


#endif
