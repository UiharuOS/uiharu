; UIHARU::MBR
;---------------------------------------------------------------------------
; nasm编译器中的预处理指令, 在编译器编译前, 包含boot.inc文件
%include "boot.inc"
SECTION MBR vstart=0x7c00
  mov ax, cs
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov fs, ax
  mov sp, 0x7c00 ; 栈指针寄存器指向0x7c00, 作为MBR栈
  mov ax, 0xb800 ; 显卡寄存器地址0xb8000, 实模式寻址, 段基址: 0xb800
  mov gs, ax
;---------------------------------------------------------------------------
; 清屏操作
; INT 0x10  功能号: 0x06  功能描述: 上卷窗口
;---------------------------------------------------------------------------
  mov ax, 0x600  ; bios 0x10中断的功能号
  mov bx, 0x700
  mov cx, 0      ; 左上角: (0, 0)
  mov dx, 0x184f ; VGA[文本模式]中, 一行容纳80个字符, 共25行
                 ; 下标从0开始, 0x18=24, 0x4f=79
                 ; 由于x86CPU是[小端模式] -> 0x184f

  int 0x10       ; 使用INT 0x10

; PRINT BG-COLOR GREEN, FR-COLOR RED, FLASH STRING "UIHARU"
;---------------------------------------------------------------------------
mov byte [gs:0x00], 'U'
mov byte [gs:0x01], 0xA4

mov byte [gs:0x02], 'i'
mov byte [gs:0x03], 0xA4

mov byte [gs:0x04], 'h'
mov byte [gs:0x05], 0xA4

mov byte [gs:0x06], 'a'
mov byte [gs:0x07], 0xA4

mov byte [gs:0x08], 'r'
mov byte [gs:0x09], 0xA4

mov byte [gs:0x0A], 'u'
mov byte [gs:0x0B], 0xA4

mov eax, LOADER_START_SECTOR    ; loader在硬盘上的LBA地址: 0x2, 第二块扇区
mov bx, LOADER_BASE_ADDR        ; loader在内存中的位置: 0x900
mov cx, 1                       ; 待读入的扇区数
call rd_disk_m_16               ; call调用函数: 从磁盘中读加载器到内存

jmp LOADER_BASE_ADDR            ; 跳转访问加载器(栈)

; 函数: rd_disk_m_16
; -> 功能: 读取硬盘的n个扇区
; --> eax: LBA的扇区号
; --> bx:  将数据写入的内存地址
; --> cx:  读入的扇区数
;---------------------------------------------------------------------------
rd_disk_m_16:
    mov esi, eax    ; 备份eax
    mov di, cx      ; 备份cx
; 读写硬盘:
; 设置要读取的扇区数
    mov dx, 0x1f2
    mov al, cl      ; 为什么使用cl寄存器?
    out dx, al      ; 读取的扇区数

    mov eax, esi    ; 恢复eax

    ;lbalow 7~0
    mov dx, 0x1f3
    out dx, al      ; 写入lbalow寄存器

    ;lbamid 15-8
    mov cl, 8
    shr eax, cl     ; 扇区起始地址shr(右移8位)
    mov dx, 0x1f4
    out dx, al

    ;lbahigh 23-16
    shr eax, cl
    mov dx, 0x1f5
    out dx, al

    ;lba device
    shr eax, cl
    and al, 0x0f    ; 按位与
    or al, 0xe0     ; 按位或
    mov dx, 0x1f6   ; device寄存器的低4位
    out dx, al

    mov dx, 0x1f7   ; 初始化状态寄存器~
    mov al, 0x20    ; 00100000~ DRDY: 设备就绪等待指令
    out dx, al

; 检测硬盘状态: BSY位
  .not_ready:
    nop             ; 空操作, 类似sleep
    in al, dx       ; 读取状态寄存器(无需重新初始化dx寄存器)
    and al, 0x88    ; 10001000 检测BSY和DRQ
    cmp al, 0x08    ; 00001000 检测BSY位
    jnz .not_ready  ; 判断结果是否为0, 得出此时的硬盘工作状态

; 从0x1f0端口(primary通道的data寄存器)读数据
    mov ax, di      ; di<-cx<-待读入的扇区数
    mov dx, 256     ; (cx*512)/2<-一次in读取2个字节: in操作的次数
    mul dx          ; 乘法操作, dx是操作数, 另一个操作数隐含在ax或al寄存器中
    mov cx, ax      ; 将ax的低16位移入cx中, 作为循环计数

    mov dx, 0x1f0   ; 访问0x1f0寄存器读取数据

  .go_on_read:
    in ax, dx
    mov [bx], ax    ; 把加载器写入内存: bx存放加载器所在的内存地址: 0x900
    add bx, 2       ; 移动2个字节
    loop .go_on_read; 循环
    ret             ; 函数返回, call函数的时候会将返回地址压入栈中

times (512-2)-($-$$) db 0
db 0x55, 0xaa
