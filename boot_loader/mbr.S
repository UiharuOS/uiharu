;---------------------------------------------------------------------------
; UIHARU::MBR E bootloader (512字节)
;---------------------------------------------------------------------------
%include "boot.inc"
SECTION MBR vstart=0x7c00 ; bios把mbr加载到0x7c00处~31KB
; 初始化各寄存器, 段寄存器需要依靠通用寄存器初始化
  mov ax, cs     ; code segment: cs寄存器CPU启动的时候就初始化过了0xf000
  mov ds, ax     ; generally points at segment where variables are defined
  mov es, ax     ; extra segment register, it's up to a coder to define its usage
  mov fs, ax     ; fs,gs are commonly used by OS kernels to access thread-specific memory
  mov ss, ax     ; stack segment register: 把整个实模式下0~0x7c00内存看作一个大栈
  mov sp, 0x7c00 ; 栈指针寄存器指向0x7c00
  mov ax, 0xb800 ; 显卡寄存器地址0xb8000, 实模式寻址会左移16位<-0xb800
  mov gs, ax     ; gs显卡基址, 通过gs可以操作显卡
;---------------------------------------------------------------------------
; 清屏操作
; INT 0x10  功能号: 0x06  功能描述: 上卷窗口
;---------------------------------------------------------------------------
  mov ax, 0600h  ; bios 0x10中断第0x600号功能
  mov cx, 0      ; 左上角: (0, 0)
  mov dx, 0x184f ; VGA[文本模式]中, 一行容纳80个字符, 共25行
                 ; 下标从0开始, 0x18=24, 0x4f=79
                 ; 由于x86CPU是[小端模式] -> 0x184f
                 ; 左上->右下: clear screen
  int 0x10       ; INT 0x10
;---------------------------------------------------------------------------
; 操作显卡输出字符
; PRINT BG-COLOR GREEN, FR-COLOR RED, FLASH STRING "16bit"
; 输出16bit表示当前处于CPU16位实模式
;---------------------------------------------------------------------------
mov byte [gs:0x00], '1'
mov byte [gs:0x01], 0xA4

mov byte [gs:0x02], '6'
mov byte [gs:0x03], 0xA4

mov byte [gs:0x04], 'b'
mov byte [gs:0x05], 0xA4

mov byte [gs:0x06], 'i'
mov byte [gs:0x07], 0xA4

mov byte [gs:0x08], 't'
mov byte [gs:0x09], 0xA4

mov eax, LOADER_START_SECTOR    ; loader在硬盘上的LBA地址: 0x2, 第3块扇区
mov bx, LOADER_BASE_ADDR        ; loader在内存中的位置: 0x900
mov cx, 4                       ; 待读入的扇区数(2048byte=2KB=4*512byte <- loader)
call rd_disk_m_16               ; call调用函数: 从磁盘中读加载器到内存

; jmp LOADER_BASE_ADDR          ; 跳转访问加载器(栈)
jmp LOADER_BASE_ADDR + 0x300    ; 使用绝对内存大小跳转
; LOADER_BASE_ADDR + 0x300 是loader_start的地址

;---------------------------------------------------------------------------
; 函数: rd_disk_m_16
; -> 功能: 读取硬盘的n个扇区
; --> eax: 加载器的LBA扇区号
; --> bx:  将数据写入的内存地址
; --> cx:  读入的扇区数
;---------------------------------------------------------------------------
rd_disk_m_16:
    mov esi, eax    ; 备份eax
    mov di, cx      ; 备份cx
; 读写硬盘:
; step1->设置要读取的扇区数
    mov dx, 0x1f2   ; 0x1f2: primary通道, sector count寄存器
    mov al, cl      ; cl: the least significant 8 bits of the cx
    out dx, al      ; 读取的扇区数

    mov eax, esi    ; 恢复eax
; step2->set lba address
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
; step3->set device register
    ;lba device
    shr eax, cl
    or al, 0xe0     ; 按位或
    mov dx, 0x1f6   ; device寄存器的低4位
    out dx, al

    mov dx, 0x1f7   ; 初始化状态寄存器~
    mov al, 0x20    ; 0010 0000~ DRDY: 设备就绪等待指令
    out dx, al      ; 写操作, command寄存器, 硬盘开始工作
; step4->read data
; 检测硬盘状态: BSY位
  .not_ready:
    nop             ; 空操作, 类似sleep
    in al, dx       ; 读取状态寄存器(无需重新初始化dx寄存器)
    and al, 0x88    ; 10001000 检测BSY和DRQ
    cmp al, 0x08    ; 00001000 检测BSY位
    jnz .not_ready  ; 判断结果是否为0, 得出此时的硬盘工作状态

; 从0x1f0端口(primary通道的data寄存器)读数据
    mov ax, di      ; di<-cx<-待读入的扇区数
    mov dx, 256     ; (512)/2<-一次in读取2个字节: 每个扇区in操作的次数
    mul dx          ; 乘法操作, dx是操作数, 另一个操作数隐含在ax或al寄存器中
                    ; -> [待读入的扇区数*每个扇区in操作的次数] = 4*256 = 1024
                    ; -> 16位乘法32位乘积的低16位在ax寄存器, 高16位在dx寄存器
    mov cx, ax      ; cx作为loop循环计数
                    ; -> ax: 1024, 1024次循环,每次2个字节,正好2048->4个扇区,能够完全读取mbr

    mov dx, 0x1f0   ; 访问0x1f0寄存器读取数据

  .go_on_read:
    in ax, dx       ; 从Primary通道主盘读数据
    mov [bx], ax    ; 把加载器写入内存: bx存放加载器所在的内存地址: 0x900
    add bx, 2       ; 移动2个字节
    loop .go_on_read; 循环
    ret             ; 函数返回, call函数的时候会将返回地址压入栈中

times (512-2)-($-$$) db 0 ; 填充0, 保证第一个扇区510字节
db 0x55, 0xaa       ; db:(define byte), 最后2位0xaa55
