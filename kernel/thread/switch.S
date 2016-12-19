[bits 32]
section .text
global switch_to
switch_to:
    ; 中断处理程序, 调用switch_to, 根据ABI
    ; 保存主调函数(内核中断处理例程)的上下文
    ; 此处为返回地址
    ; 第一次执行, kthread_stack的地址, 返回执行kthread_stack
    ; 不是第一次被调用, 主调函数(内核中断例程)的地址, 返回继续执行未执行完的内核代码
    push esi
    push edi
    push ebx
    push ebp
    mov eax, [esp+20] ; current
    mov [eax], esp    ; 保存当然内核栈指针到current pcb
                      ; 偏移为0处的thread_kstack
                      ; 内核线程栈就是保存寄存器映像的栈位于PCB中
    ; 恢复下一个线程的环境
    mov eax, [esp+24] ; next
    mov esp, [eax]    ; 恢复next线程的内核栈为当前调用栈
    ; ABI: 恢复**next先前被调用时**,
    ; 存储在next->thread_kstack栈中的主调上下文环境
    pop ebp
    pop ebx
    pop edi
    pop esi
    ret
