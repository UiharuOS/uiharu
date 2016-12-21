#include "sync.h"
#include "type.h"
#include "interrupt.h"
#include "debug.h"
#include "thread.h"

void semaphore_init(struct semaphore* usemaphore, uint8_t value) {
    /* semaphore_init 初始化信号量 */
    usemaphore->value = value;
    list_init(&usemaphore->waiters);
}

void lock_init(struct lock* ulock) {
    /* lock_init 初始化锁 */
    ulock->guard = NULL;
    ulock->guard_repeat_num = 0;
    // 用二元信号量实现锁
    semaphore_init(&ulock->lock_semaphore, 1);
}

void udown(struct semaphore* usemaphore) {
    /* udown: 对信号量进行down操作(P,V操作中的P) */
    enum intr_status old_status = intr_disable();
    // 关中断
    while (usemaphore->value == 0) {
        // 如果信号量已经为0, 表示锁已经被别的什么家伙拿走了
        // 那我只有休息便是了
        struct task_struct* current = get_running_thread_pcb();
        ASSERT(!elem_find(&usemaphore->waiters, &current->thread_elem));
        if (elem_find(&usemaphore->waiters, &current->thread_elem)) {
            DEBUGGER("udown: thread blocked already in semaphore waiters list\n");
        } else {
            list_append(&usemaphore->waiters, &current->thread_elem);
            uthread_sleep(TASK_BLOCKED);  // 将当前线程置为阻塞状态, 直到被唤醒
        }
    }
    usemaphore->value--;  // 获取锁
    ASSERT(usemaphore->value == 0);
    // 恢复中断
    intr_set_status(old_status);
}

void uup(struct semaphore* usemaphore) {
    /* uup 信号量的up操作(P,V操作里面的V操作) */
    enum intr_status old_status = intr_disable();
    // 关闭中断
    ASSERT(usemaphore->value == 0);
    if (!list_empty(&usemaphore->waiters)) {
        struct list_entry* sleep_thread_tag = list_pop(&usemaphore->waiters);
        struct task_struct* sleep_thread = elem2struct(struct task_struct, thread_elem, sleep_thread_tag);
        uthread_awake(sleep_thread); // 唤醒
    }
    usemaphore->value++;
    ASSERT(usemaphore->value == 1);
    // 恢复中断
    intr_set_status(old_status);
}
