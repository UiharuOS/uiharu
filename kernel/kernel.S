;------------------------------------------
; [intel汇编]中断处理程序
;------------------------------------------
[bits 32]
%define ERROR_CODE nop  ; 如果产生异常,相关栈中压入了错误码,就不做操作
%define ZERO push 0     ; 如果产生异常,CPU没有把错误码压入栈中,则压入0,保持ESP位置

extern print_string

section .data
intr_str db "interrupt occur!", 0xa, 0
global intr_entry_table ; 中断向量数组(向量号下标,对应着一段中断处理例程)
intr_entry_table:

%macro VECTOR 2
section .text
; 中断处理程序
intr%1entry:            ; 每个标号对应着一个中断向量号->中断处理程序
    %2
    push intr_str
    call print_string   ; 调用字符串打印函数
    add esp, 4          ; 跳过参数(2个参数)

    ; 如果是从[从片]上进入的中断, 除了往[从片]上发送EOI外, 还要往[主片]上发送EOI
    ; 因为CPU接受中断, 返回中断响应, 都是和主片进行交互的
    mov al, 0x20        ; EOI命令8259A的OCW2控制
    out 0xa0, al        ; 向从片发送EOI
    out 0x20, al        ; 向主片发送EOI

    add esp, 4          ; 跨过error_code
    iret                ; 从中断返回(可能会有特权级的变化)

section .data
    dd intr%1entry      ; 存储中断入口程序地址
                        ; 这些数据段最终会被合并, 从而
                        ; 构造intr_entry_table数组
%endmacro

; 构建33个中断向量处理程序
VECTOR 0x00, ZERO
VECTOR 0x01, ZERO
VECTOR 0x02, ZERO
VECTOR 0x03, ZERO
VECTOR 0x04, ZERO
VECTOR 0x05, ZERO
VECTOR 0x06, ZERO
VECTOR 0x07, ZERO
VECTOR 0x08, ZERO
VECTOR 0x09, ZERO
VECTOR 0x0a, ZERO
VECTOR 0x0b, ZERO
VECTOR 0x0c, ZERO
VECTOR 0x0d, ZERO
VECTOR 0x0e, ZERO
VECTOR 0x0f, ZERO
VECTOR 0x10, ZERO
VECTOR 0x11, ZERO
VECTOR 0x12, ZERO
VECTOR 0x13, ZERO
VECTOR 0x14, ZERO
VECTOR 0x15, ZERO
VECTOR 0x16, ZERO
VECTOR 0x17, ZERO
VECTOR 0x18, ZERO
VECTOR 0x19, ZERO
VECTOR 0x1a, ZERO
VECTOR 0x1b, ZERO
VECTOR 0x1c, ZERO
VECTOR 0x1d, ZERO
VECTOR 0x1e, ERROR_CODE
VECTOR 0x1f, ZERO
VECTOR 0x20, ZERO
