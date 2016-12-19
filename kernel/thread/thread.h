#ifndef KERNEL_THREAD_H
#define KERNEL_THREAD_H

#include "stdint.h"

/* 自定义通用函数类型thread_func,作为线程函数的形参类型 */
typedef void thread_func(void*);

/* 进程(线程)状态 */
enum task_status {
    TASK_RUNNING,   // 运行态
    TASK_READY,     // 就绪态
    TASK_BLOCKED,   // 阻塞态
    TASK_WAITING,   // 等待态
    TASK_HANGING,   // 挂起态
    TASK_DIED       // 死了
};

/* 中断栈:中断发生时保护进程的上下文环境 */
struct intr_stack {
    // 保护模式压入32位寄存器
    uint32_t vec_no;    // 中断向量号
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy; // esp(栈指针)是不断变化的
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
    // CPU从低特权级进入高特权级时压入
    uint32_t err_code;
    void (*eip) (void);
    uint32_t cs;
    uint32_t eflags;
    void* esp;
    uint32_t ss;
};

/* 线程栈:线程运行时环境 */
struct thread_stack {
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;
    void (*eip) (thread_func* func, void* func_args);
    void (*unused_retaddr);  // 用于在返回地址所在的栈帧中占位
    thread_func* function;   // 线程中执行的函数
    void* func_args;         // 线程中执行函数的参数
};

/* Process Control Block */
struct task_struct {
    uint32_t* self_kstack;   // 内核线程自己的内核栈
    enum task_status status; // 进程状态
    uint8_t priority;        // 线程调度优先级
    char name[16];
    uint32_t stack_magic;
};

// thread functions
static void kernel_thread(thread_func* function, void* func_args);
void thread_create(struct task_struct* pthread, thread_func function, void* func_args);
void init_thread(struct task_struct* pthread, char* name, int priority);
struct task_struct* thread_start(char* name,           \
                                 int priority,         \
                                 thread_func function, \
                                 void* func_args);

#endif
