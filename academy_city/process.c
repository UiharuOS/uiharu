#include "global.h"
#include "thread.h"
#include "print.h"
#include "memory.h"
#include "debug.h"
#include "userproc.h"

# 引用kernel.S中的中断退出函数
extern void intr_exit(void);

void start_process(void* _filename) {
    /* start_process: 构建用户进程初始上下文信息 */
    void* function = _filename;
    struct task_struct* current = get_running_thread_pcb();
    current->self_kstack += sizeof(struct thread_stack);
    struct intr_stack* proc_stack = (struct intr_stack*)current->self_kstack;
    /*               ----------------
     *              |   intr_stack   |
     * self_kstack ->----------------<- proc_stack
     *              |  thread_stack  |
     *               ----------------
     *              |PCB(task_struct)|
     *               ----------------
     *              0000000stack000000
     */
    proc_stack->edi = proc_stack->esi =\
    proc_stack->ebp = proc_stack->esp_dummy = 0;
    proc_stack->ebx = proc_stack->edx =\
    proc_stack->ecx = proc_stack->eax = 0;
    proc_stack->gs = 0;  // uiharu中, 显存只允许内核操作
    proc_stack->ds = proc_stack->es = proc_stack->fs = SELECTOR_U_DATA;
    proc_stack->eip = function; // 待执行的用户程序的地址
    proc_stack->cs = SELECTOR_U_CODE;  // RPL3, cs.rpl为3, 从中断返回后就会进入到3特权级下
    proc_stack->eflags = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);
    proc_stack->esp = (void*)( /* 从用户内存池拿到一物理页, 并分配USER_STACK3_VADDR为虚拟地址 */
                      (uint32_t)get_a_page(PF_USER, USER_STACK3_VADDR)+PG_SIZE);
    proc_stack->ss = SELECTOR_U_DATA;
    asm volatile (
                 "movl %0, %%esp; jmp intr_exit"
                 : /* no outputs */
                 : "g" (proc_stack) // any register, memory or immediate integer operand is allowed
                 : "memory"
                 );
}

void page_dir_activate(struct task_struct* thread) {
    /* 激活页表 */
    uint32_t pagedir_phy_addr = 0x100000;
    if (thread->pgdir != NULL) {
        // 用户进程, 拥有自己的地址空间(页表)
        pagedir_phy_addr = vaddr2phy((uint32_t)thread->pgdir);
    }
    asm volatile (// 更新cr3, 使新页表生效
                 "movl %0, %%cr3"
                 : /* no outputs */
                 : "r" (pagedir_phy_addr)
                 : "memory"
                 );
}

void process_activate(struct task_struct* thread) {
    /* process_activate: 更新任务的页表,更新tss中esp0为任务的内核栈 */
    ASSERT(thread != NULL);
    page_dir_activate(thread);
    if (thread->pgdir) {
        // 利用tss做多任务切换, 不过不是每个任务一个tss,
        // 而是所有任务公用一个tss, 不同任务加载不同的esp和环境
        // CPU看到的tss并没有切换, 执行场所、环境变化从而实现任务切换
        update_tss_esp(thread);
    }
}
