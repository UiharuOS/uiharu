%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR
jmp loader_start

; 构建gdt及其内部描述符
GDT_BASE: dd 0x00000000
          dd 0x00000000
CODE_BASE: dd 0x0000FFFF
           dd DESC_CODE_HIGH4
DATA_STACK_DESC: dd 0x0000FFFF
                 dd DESC_DATA_HIGH4
VIDEO_DESC: dd 0x80000007 ; limit=(0xbffff-0xb8000)/4k=0x7
            dd DESC_VIDEO_HIGH4
GDT_SIZE equ $ - GDT_BASE
GDT_LIMIT equ GDT_SIZE - 1
times 60 dq 0  ; 此处预留60个描述符空位
SELECTOR_CODE equ (0x0001<<3) + TI_GDT + RPL0
SELECTOR_DATA equ (0x0002<<3) + TI_GDT + RPL0
SELECTOR_VIDEO equ (0x0003<<3)+ TI_GDT + RPL0
; 以下是gdt的指针, 前2字节是gdt界限, 后4字节是gdt起始地址
gdt_ptr dw GDT_LIMIT
        dd GDT_BASE
loadermsg db 'Hello UiharuOS!'

loader_start:
;-------------------------------------------------------
; bios 0x10中断 打印字符串
;-------------------------------------------------------
mov sp, LOADER_BASE_ADDR
mov bp, loadermsg
mov cx, 15
mov ax, 0x1301
mov bx, 0x001f
mov dx, 0x0C1F
int 0x10

; 准备进入保护模式
; -> 1 打开A20
; -> 2 加载gdt
; -> 3 将cr0的pe位置为1

; 1 打开A20地址总线
in al, 0x92
or al, 0000_0010b
out 0x92, al
; 2 加载gdt
lgdt [gdt_ptr]
; 3 将cr0的pe位置为1
mov eax, cr0
or eax, 0x00000001
mov cr0, eax

jmp dword SELECTOR_CODE:p_mode_start  ; 刷新流水线

[bits 32]
p_mode_start:
    mov ax, SELECTOR_DATA
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, LOADER_STACK_TOP
    mov ax, SELECTOR_VIDEO
    mov gs, ax

    ; mov byte [gs:160], 'P'

jmp $
