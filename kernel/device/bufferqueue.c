#include "bufferqueue.h"
#include "interrupt.h"
#include "sync.h"

void bufferqueue_init(struct bufferqueue* bufq) {
    lock_init(&bufq->lock);
    bufq->current_producer = bufq->current_consumer = NULL;
    bufq->head->tail = 0;
}

int32_t next_pos(int32_t pos) {
    return (pos+1) % buffer_size;
}

boolean bufferqueue_full(struct bufferqueue* bufq) {
    return (next_pos(bufq->head) == bufq->tail);
}

boolean bufferqueue_empty(struct bufferqueue* bufq) {
    return (bufq->tail == bufq->head);
}

static void bufferqueue_sleep(struct task_struct** waiter) {
    ASSERT(*waiter == NULL && waiter != NULL);
    *waiter = get_running_thread_pcb();
    uthread_sleep(TASK_BLOCKED);
}

static void bufferqueue_awake(struct task_struct** waiter) {
    ASSERT(*waiter != NULL);
    uthread_awake(*waiter);
    *waiter = NULL;
}

char bufferqueue_getchar(struct bufferqueue* bufq) {
    ASSERT(intr_get_status() == INTR_OFF);
    while (bufferqueue_empty(bufq)) {
        lacquire(&bufq->lock);
        bufferqueue_sleep(&bufq->current_consumer);
        lrelease(&bufq->lock)
    }
    char byte = bufq->buffer[bufq->tail];
    bufq->tail = next_pos(bufq->tail);
    if (bufq->current_producer != NULL) {
        bufferqueue_awake(&bufq->current_producer);
    }
    return byte;
}

void  bufferqueue_putchar(struct bufferqueue* bufq, char byte) {
    ASSERT(intr_get_status() == INTR_OFF);
    while (bufferqueue_full(bufq)) {
        lacquire(&bufq->lock);
        bufferqueue_sleep(&bufq->current_producer);
        lrelease(&bufq->lock)
    }
    bufq->buffer[bufq->head] = byte;
    bufq->head = next_pos(bufq->head);
    if (bufq->current_consumer != NULL) {
        bufferqueue_awake(&bufq->current_consumer);
    }
}
