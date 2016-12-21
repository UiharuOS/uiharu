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

#endif
