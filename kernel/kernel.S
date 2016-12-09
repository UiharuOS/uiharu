;-----------------------------------------------------
; [intel汇编]中断处理程序
; (处理程序的主体直接调用kernel/interrupt.c中的中断处理程序)
; 此汇编是入口, 保存上下文->(调用中断处理程序)->恢复上下文
;-----------------------------------------------------
[bits 32]
%define ERROR_CODE nop  ; 如果产生异常,相关栈中压入了错误码,就不做操作
%define ZERO push 0     ; 如果产生异常,CPU没有把错误码压入栈中,则压入0,保持ESP位置

; 引用外部对象(c)
; extern print_string
extern idt_table        ; interrupt.c 中定义和初始化的中断处理程序数组

section .data
global intr_entry_table ; 中断向量数组(向量号下标,对应着一段中断处理例程)
intr_entry_table:

%macro VECTOR 2
section .text
; 中断处理程序
intr%1entry:            ; 每个标号对应着一个中断向量号->中断处理程序
    %2
    ; 保存上下文环境
    push ds ; generally points at segment where variables are defined
    push es ; extra segment register, it's up to a coder to define its usage
    push fs ; fs, gs are commonly used by OS kernels to access thread-specific memory
    push gs ; http://stackoverflow.com/questions/10810203/what-is-the-fs-gs-register-intended-for
    pushad  ; pushad指令压入32位寄存器, 入栈顺序是: eax, ecx, edx, ebx, esp, ebp, esi, edi
    
    ; 如果是从[从片]上进入的中断, 除了往[从片]上发送EOI外, 还要往[主片]上发送EOI
    ; 因为CPU接受中断, 返回中断响应, 都是和主片进行交互的
    mov al, 0x20        ; EOI命令8259A的OCW2控制
    out 0xa0, al        ; 向从片发送EOI
    out 0x20, al        ; 向主片发送EOI

    push %1 ; 压入中断向量号, 与中断处理程序对应, 方便异常处理

    call [idt_table + (%1)*4] ; 调用idt_table中C版本的中断处理函数, 内存寻址, 调用该内存段上的中断处理程序
    jmp intr_exit

section .data
    dd intr%1entry      ; 存储中断入口程序地址
                        ; 这些数据段最终会被合并, 从而
                        ; 构造intr_entry_table数组
%endmacro

section .text
global intr_exit
intr_exit:
; 恢复上下文环境
    add esp, 4          ; 跳过中断号
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 4          ; 跳过ErrorCode
    iretd

; 构建33个中断向量处理程序
VECTOR 0x00,ZERO
VECTOR 0x01,ZERO
VECTOR 0x02,ZERO
VECTOR 0x03,ZERO
VECTOR 0x04,ZERO
VECTOR 0x05,ZERO
VECTOR 0x06,ZERO
VECTOR 0x07,ZERO
VECTOR 0x08,ZERO
VECTOR 0x09,ZERO
VECTOR 0x0a,ZERO
VECTOR 0x0b,ZERO
VECTOR 0x0c,ZERO
VECTOR 0x0d,ZERO
VECTOR 0x0e,ZERO
VECTOR 0x0f,ZERO
VECTOR 0x10,ZERO
VECTOR 0x11,ZERO
VECTOR 0x12,ZERO
VECTOR 0x13,ZERO
VECTOR 0x14,ZERO
VECTOR 0x15,ZERO
VECTOR 0x16,ZERO
VECTOR 0x17,ZERO
VECTOR 0x18,ZERO
VECTOR 0x19,ZERO
VECTOR 0x1a,ZERO
VECTOR 0x1b,ZERO
VECTOR 0x1c,ZERO
VECTOR 0x1d,ZERO
VECTOR 0x1e,ERROR_CODE
VECTOR 0x1f,ZERO
VECTOR 0x20,ZERO