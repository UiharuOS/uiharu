#include "thread.h"
#include "stdint.h"
#include "string.h"
#include "global.h"
#include "memory.h"

#define PG_SIZE 4096

static void kernel_thread(thread_func* function, void* func_args) {
    /* kernel_thread: 由内核线程执行function(func_args) */
    function(func_args);
}

void thread_create(struct task_struct* pthread, thread_func function, void* func_args) {
    /* thread_create: 初始化线程栈:thread_stack */
    // 在线程内核栈中预留中断使用的栈空间
    pthread->self_kstack -= sizeof(struct intr_stack);
    // 在线程内核栈中预留线程使用的栈空间
    pthread->self_kstack -= sizeof(struct thread_stack);
    // 初始化线程栈
    struct thread_stack* kthread_stack = (struct thread_stack*)pthread->self_kstack;
    kthread_stack->eip = kernel_thread;  // 第一次执行时eip会指向kernel_thread
    kthread_stack->function = kernel_thread;
    kthread_stack->func_args = func_args;
    kthread_stack->ebp = kthread_stack->ebx = \
    kthread_stack->esi = kernel_thread->edi = 0; // 全部初始化为0, 会被被调函数保存到线程栈中
}

void init_thread(struct task_struct* pthread, char* name, int priority) {
    /* int_thread: 初始化进程(线程)的基本信息 */
    memset(pthread, 0, sizeof(*pthread));
    strcpy(pthread->name, name);
    pthread->status = TASK_RUNNING;
    pthread->priority = priority;
    // self_kstack是该线程在内核态下使用的栈顶地址
    // uiharu进程PCB占连续的一个物理页, 线程栈位于PCB顶部位置(高地址)
    pthread->self_kstack = (uint32_t*)((uint32_t)pthread + PG_SIZE);
    pthread->stack_magic = 0x19960411;
}

struct task_struct* thread_start(char* name,           \
                                 int priority,         \
                                 thread_func function, \
                                 void* func_args) {
    /* thread_start: 创建一个指定优先级的线程 */
    // 所有进程(kernel/user)的PCB都由操作系统调度维护, 位于内核空间
    struct task_struct* thread = get_kernel_pages(1);  // 在内核物理内存池中申请一个物理页...
    init_thread(thread, name, priority);
    thread_create(thread, function, func_args);

    asm volatile ("movl %0, %%esp;\
                   pop %%ebp; \
                   pop %%ebx; \
                   pop %%edi; \
                   pop %%esi; \
                   ret"
                  :
                  : "g" (thread->self_kstack)
                  : "memory");
    return thread;
}
