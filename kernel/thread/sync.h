#ifndef KERNEL_THREAD_SYNC_H
#define KERNEL_THREAD_SYNC_H

#include "dlist.h"
#include "stdint.h"
#include "thread.h"

/* 信号量 */
struct semaphore {
    uint8_t value;        // 信号量
    struct list waiters;  // 等待队列
};

/* 锁结构 */
struct lock {
    struct task_struct* guard;       // 拥有锁的人->门卫
    struct semaphore lock_semaphore; // 信号量, 会被赋初值为1, 二元信号量实现锁
    uint32_t guard_repeat_num;      // 锁的持有者重复申请锁的次数
};

void semaphore_init(struct semaphore* usemaphore, uint8_t value);
void lock_init(struct lock* ulock);
void udown(struct semaphore* usemaphore);  // 原子操作->信号量P操作
void uup(struct semaphore* usemaphore);    // 原子操作->信号量V操作
void lacquire(struct lock*);
void lrelease(struct lock*);

#endif
