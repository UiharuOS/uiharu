#include "thread.h"
#include "stdint.h"
#include "string.h"
#include "global.h"
#include "memory.h"
#include "dlist.h"
#include "debug.h"
#include "interrupt.h"
#include "print.h"
#include "type.h"

#define PG_SIZE 4096

struct task_struct* main_thread;    // 主线程pcb
struct list thread_ready_list;      // 就绪任务队列
struct list thread_all_list;        // 所有任务队列
static struct list_entry* general_thread_elem;
/* switch from current to next */
extern void switch_to(struct task_struct* current, struct task_struct* next);

struct task_struct* get_running_thread_pcb() {
    /* get_running_thread_pcb */
    uint32_t esp;
    asm ("mov %%esp, %0"
        : "=g" (esp)
        : /* no input */
        : /* no clobber */
        );
    // 线程pcb(页表ps地址空间)的虚拟地址, high 20bit
    // address space -> pte -> 4kb
    return (struct task_struct*)(esp & 0xfffff000);
}

static void kernel_thread(thread_func* function, void* func_args) {
    /* kernel_thread: 由内核线程执行function(func_args) */
    intr_enable();  // 开中断,时钟中断调度
    function(func_args);
}

void thread_create(struct task_struct* thread, thread_func function, void* func_args) {
    /* thread_create: 初始化线程栈:thread_stack */
    thread->self_kstack -= sizeof(struct intr_stack);
    thread->self_kstack -= sizeof(struct thread_stack);
    // 初始化线程栈
    struct thread_stack* kthread_stack = (struct thread_stack*)thread->self_kstack;

    kthread_stack->eip = kernel_thread;  // 第一次执行时eip会指向kernel_thread
    kthread_stack->function = function;
    kthread_stack->func_args = func_args;
    kthread_stack->ebp = kthread_stack->ebx = \
    kthread_stack->esi = kthread_stack->edi = 0; // 全部初始化为0, 会被被调函数保存到线程栈中
}

void init_thread(struct task_struct* thread, char* name, int priority) {
    /* int_thread: 初始化进程(线程)的基本信息 */
    memset(thread, 0, sizeof(*thread)); // 物理页清0, 避免分配到脏页, get_kernel_page
    strcpy(thread->name, name);
    if (thread == main_thread) {
        // 主线程, task status
        thread->status = TASK_RUNNING;
    } else {
        // 其他线程
        thread->status = TASK_READY;
    }
    // self_kstack是该线程在内核态下使用的栈顶地址
    // uiharu进程PCB占连续的一个物理页, 线程栈位于PCB顶部位置(高地址)
    // 所以是[起始虚拟地址+PG_SIZE], (uint32_t)pthread就是pthread pcb的起始地址,
    // a little trick:), stack downward
    thread->self_kstack = (uint32_t*)((uint32_t)thread + PG_SIZE);
    thread->priority = priority;
    thread->ticks = priority; // 调度策略: 优先级越大, CPU分与的时间片越长
    thread->all_ticks = 0;
    thread->pgdir = NULL;
    thread->stack_magic = 0x19960411;  // 这是一个神奇的数字, 用作魔数
}

struct task_struct* uthread_create(char* name,           \
                                   int priority,         \
                                   thread_func function, \
                                   void* func_args) {
    /* uthread_create: 创建一个指定优先级的线程 */
    // 所有进程(kernel/user)的PCB都由操作系统调度维护, 位于内核空间
    // 在内核物理内存池中申请一页内存分配给该线程(所属进程)的PCB
    // thread存储的是该页内存的起始虚拟地址
    struct task_struct* thread = get_kernel_pages(1);
    init_thread(thread, name, priority);
    thread_create(thread, function, func_args); // function(func_args)

    // 线程调度队列
    // 被调度线程之前不在线程队列中
    // fuck shit ASSERT!!! (xiaozheku)
    ASSERT(!elem_find(&thread_all_list, &thread->thread_all_list_elem));
    // 将该线程加入线程队列中
    list_append(&thread_all_list, &thread->thread_all_list_elem);
    // 被调度线程之前不在就绪队列中
    ASSERT(!elem_find(&thread_ready_list, &thread->thread_elem));
    // 将该线程加入就绪队列中
    list_append(&thread_ready_list, &thread->thread_elem);

    return thread;
}

void uthread_sleep(enum task_status status) {
    ASSERT(((status == TASK_BLOCKED) || \
            (status == TASK_WAITING) || \
            (status == TASK_HANGING)));
    enum intr_status old_status = intr_disable();
    // 关中断
    struct task_struct* current = get_running_thread_pcb();
    current->status = status;
    schedule();  // 将当前线程换下CPU
    // 恢复中断
    intr_set_status(old_status);
}

void uthread_awake(struct task_struct* thread) {
    enum intr_status old_status = intr_disable();
    enum task_status status = thread->status;
    // 关中断
    ASSERT(((status == TASK_BLOCKED) || \
            (status == TASK_WAITING) || \
            (status == TASK_HANGING)));
    if (thread->status != TASK_READY) {
        ASSERT(status != TASK_DIED);
        ASSERT(!elem_find(&thread_ready_list, &thread->thread_elem));
        if (elem_find(&thread_ready_list, &thread->thread_elem)) {
            DEBUGGER("blocked thread in ready list\n");
        } else {
            thread->status = TASK_READY;
            list_push(&thread_ready_list, &thread->thread_elem);
        }
    }
    // 恢复中断
    intr_set_status(old_status)
}

static void make_main_thread(void) {
    /* make_main_thread: 完善主线程kernel:main */
    // 内核被加载时已经初始化了地址空间
    main_thread = get_running_thread_pcb();
    init_thread(main_thread, "main", 12);
    print_string("Info)--> main_thread ");
    print_string(main_thread->name);
    print_string(" ");
    print_int(main_thread->priority, 'D');
    print_string("\n");
    // main已经运行, 不用添加到ready队列
    // add main in thread list
    ASSERT(!elem_find(&thread_all_list, &main_thread->thread_all_list_elem));
    list_append(&thread_all_list, &main_thread->thread_all_list_elem);
}

void schedule() {
    /* 任务调度: RR interrupt schedule */
    // 任务调度由时钟中断中断处理例程调用, 此时应已经关中断
    ASSERT(intr_get_status() == INTR_OFF);
    struct task_struct* current = get_running_thread_pcb();
    if (current->status == TASK_RUNNING) {
        // 当前任务是因为时间片到期被换下的, 而不是什么别的可怕原因
        ASSERT(!elem_find(&thread_ready_list, &current->thread_elem));
        // 添加到就绪队列末尾, 待下次轮询调度
        list_append(&thread_ready_list, &current->thread_elem);
        // 重置时间片
        current->ticks = current->priority;
        // 重置任务状态
        current->status = TASK_READY;
    } else {
        /* 我就笑笑不说话 */
    }
    // 如果就绪队列不为空
    ASSERT(!list_empty(&thread_ready_list));
    // 清空general_thread_elem
    general_thread_elem = 0;
    // 调度就绪队列队首任务上CPU
    general_thread_elem = list_pop(&thread_ready_list); // 保存下一个运行的任务
    // 将该任务节点转换成对应的PCB
    // (task_struct*)((int)thread_elem-(int)(&((task_struct*)0)->next->status))
    struct task_struct* next = elem2struct(struct task_struct, thread_elem, general_thread_elem);
    next->status = TASK_RUNNING;
    // 调度切换
    switch_to(current, next);
}

void thread_init(void) {
    /* 初始化线程 */
    print_string("Info)-> thread_init start\n");
    list_init(&thread_ready_list);
    list_init(&thread_all_list);
    make_main_thread();
    print_string("Info)-> thread_init done\n");
}
