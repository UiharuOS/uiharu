TI_GDT equ 0
RPL0 equ 0
SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0

[bits 32]
section .text
;------------------------------------------------
; print_char: 把栈中的一个字符写入光标所在的位置
;------------------------------------------------
global print_char   ; 导出为全局变量, 供外部文件调用
print_char:
    pushad          ; 备份32位寄存器环境
    mov ax, SELECTOR_VIDEO
    mov gs, ax      ; 为保证gs中为正确的显存段选择子
                    ; 每次打印的时候都给gs赋值
                    ; 因为用户进程越权访问可能会使CPU置gs为0, 从而在越权访问时抛出异常
; 获取当前光标位置(8位, 8位)
; 获取高8位
; -> dx索引CRT,拿到0x0e放到DataRegister中进行操作(int, out)
    mov dx, 0x03d4  ; Miscellaneous Output Register default x equ D
    mov al, 0x0e    ; Cursor Location High Register~提供光标位置的高8位
    out dx, al      ; 将当前光标高8位设为索引值
    mov dx, 0x03d5  ; 通过读写DataRegistr来获得光标位置
    in al, dx       ; al得到光标位置的高8位
    mov ah, al      ; 备份al, ax高8位为光标的高8位
; 获取低8位
; -> dx索引CRT,拿到0x0f放到DataRegister中进行操作(int, out)
    mov dx, 0x03d4
    mov al, 0x0f    ; Cursor Location Low Register~提供光标位置的低8位
    out dx, al
    mov dx, 0x03d5
    in al, dx       ; ax的低8位为光标的低8位
; 将光标存入bx
; bx <- ax = ah(igh) + al(ow)
    mov bx, ax      ; bx存放光标的位置, bx是下一个可打印字符的光标坐标值
                    ; 光标的位置是下一个可打印的字符
                    ; bs左移1位是光标在显存中的相对地址
    mov ecx, [esp + 36] ; pushad 备份32位寄存器环境压入32字节
                        ; 主调函数4字节返回地址(自动被压入栈中), 共36字节
                        ; ecx为待打印的字符参数
    ; 字符的Ascii码为1字节, cl就够了
    cmp cl, 0xd     ; CR(回车)的AsciiCode: 0x0d
    jz .is_cr
    cmp cl, 0xa     ; LF(换行)的AsciiCode: 0x0a
    jz .is_lf
    cmp cl, 0x8     ; BS(退格)的AsciiCode: 0x8
    js .is_bs

    jmp .print_other ; 否则一律视为可打印字符,打印显示:)

.is_bs:
    ;dec bx                 ; 光标坐标指向前一个字符
    ;sub bx, 2
    ;sub bx, 1
    ;sub bx, 2
    ;shl bx, 1              ; 光标左移一位, bx*2
    ;mov byte [gs:bx], 0x20 ; 将待删除的字符的低字节Ascii码设为空格Space

    ;inc bx                 ; 光标+1,指向属性位
    ;;add bx, 1
    ;mov byte [gs:bx], 0x07 ; 将待删除的字符的高字节属性设为0x07(00000111):黑屏白字
    ;;sub bx, 1
    ;shr bx, 1              ; 显存相对地址 -> 光标坐标
    ;jmp .set_cursor        ; 设置光标的位置(更新光标坐标寄存器)
    shl bx, 1
    mov byte [gs:bx], 0x00
    inc bx
    mov byte [gs:bx], 0x07
    shr bx, 1
    inc bx
    jmp .set_cursor
; 换行、回车全部设为到下一行行首
.is_lf:                     ; 换行
.is_cr:                     ; 回车
    xor dx, dx
    mov ax, bx
    mov si, 80
    div si                  ; 当前光标位置除以80的余数存放在dx寄存器中
    sub bx, dx              ; bx为当前行的行首
.is_cr_end:
    add bx, 80              ; 换行
    cmp bx, 2000
.is_lf_end:
    jl .set_cursor

.print_other:
    shl bx, 1               ; 光标位置2字节表示, 光标值乘2表示对应显存中的偏移字节
                            ; 显卡打印是逐字节设置的
    mov [gs:bx], cl         ; ASCII字符本身
    inc bx
    mov byte [gs:bx], 0x07  ; 这是字符属性
    shr bx, 1               ; 恢复成光标坐标值
    inc bx                  ; bx为新的可打印字符的坐标
    cmp bx, 2000            ; 判断有无超出一屏
    jl .set_cursor          ; 若不超过则继续设置新的光标值
                            ; 否则滚屏

; 滚屏
.roll_screen:
    cld                     ; 滚屏采用字符搬运做:)
    mov ecx, 960            ; 2000-80(去掉第1行)=1920个字符需要被搬运
                            ; 1920*2 = 3840字节
                            ; 一次搬运(32/8=4字节)
                            ; 总共搬运 3840/4=960字节, 作为rep的重复计数
    mov esi, 0xc00b80a0     ; 源地址~第1行行首(b8000h+0xa0(160字节), 虚拟地址)
    mov edi, 0xc00b8000     ; 第0行行首
    rep movsd               ; 逐字节搬运, 一次搬运4个字节[d]
    mov ebx, 3840           ; 最后一行首字符的第一个字节偏移=1920*2
    mov ecx, 80             ; 一行80字符, 每次清空1字符(2个字节), 一行需要移动80次(cx)
.cls:
    mov word [gs:ebx], 0x0720 ; 0x0720: 黑底白字空格[word直接写入2个字节]
    add ebx, 2              ; 增加2个字节(一个字符)
    loop .cls
    mov bx, 1920            ; 将光标重置为1920: 最后一行首字符

.set_cursor:
; 更新光标寄存器为bx值  
    mov dx, 0x03d4          ; Address Register
    mov al, 0x0e            ; Cursor Register High Location
    out dx, al
    mov dx, 0x03d5          ; Data Register
    mov al, bh              ; high of bx
    out dx, al              ; 设置光标寄存器的高8位

    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    mov al, bl
    out dx, al
.print_char_done:
    popad                   ; 恢复32位寄存器环境
    ret
