%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR
; jmp loader_start

;-------------------------------------------------------
; 构建gdt及其内部描述符
; -> 代码段, 数据段, 显存段
; -> GDT_BASE:00000000000000000000000000000000000000000000000000000000000 = hi:0x00000000, lo:0x00000000
; -> CODE_DESC:|00000000|1100|1111|1|00|1|1000|00000000|000000000000001111111111111111| = hi:0x00CF9800 lo:0x0000FFFF
; -> DATA_STACK_DESC: 
; -> DESC_LIMIT_VIDEO: (0x0007+ 0x0000)*4k = 0x00007*4k = 0x00007FFF
;-------------------------------------------------------
GDT_BASE: dd 0x00000000
          dd 0x00000000  ; GDT定义从0开始(相对于0x900)
CODE_DESC: dd 0x0000FFFF ; 段界限0xFFFF 0~15位, 段基址的0~15位, 平坦模式段基址都是0
           dd DESC_CODE_HIGH4 ; 段描述符的高32位
DATA_STACK_DESC: dd 0x0000FFFF
                 dd DESC_DATA_HIGH4
VIDEO_DESC: dd 0x80000007 ; 0xb800 + limit=(0xbffff-0xb8000)/4k=0x7
            dd DESC_VIDEO_HIGH4
GDT_SIZE equ $ - GDT_BASE  ; 32字节? 0x20
GDT_LIMIT equ GDT_SIZE - 1 ; 0x1F
times 60 dq 0  ; 此处预留60个描述符空位, 先用0占位(还没有被规划为gdt)
SELECTOR_CODE equ (0x0001<<3) + TI_GDT + RPL0  ; 0特权级
SELECTOR_DATA equ (0x0002<<3) + TI_GDT + RPL0
SELECTOR_VIDEO equ (0x0003<<3)+ TI_GDT + RPL0
; 保存检测出来的内存大小
; 当前偏移文件头(60+4)*8byte=512=0x200字节
; loader被加载进内存的地址是0x900
; 所以total_mem_bytes在内存中的地址是0xb00
total_mem_bytes dd 0  ; dd=>define double word(双字), 4字节大小变量
                      ; 32位地址总线最大内存4G=4*1024*1024*1024=4294967296
                      ; 4字节大小变量可以表示: 2^32=4294967296 --> 正好:)
; 以下是gdt的指针, 前2字节是gdt界限, 后4字节是gdt起始地址
; lgdt 48位操作数(6个字节)
; -- 前16位是GDT以[字节]为单位的界限值: GDT大小-1
; -- 后32位是GDT的起始地址
gdt_ptr dw GDT_LIMIT  ; 2字节 1Fh
        dd GDT_BASE   ; 4字节 0x900
; loadermsg db 'Hello UiharuOS!'

; 人工对齐:
; 定义缓冲区, 存储BIOS 0x15中断0xe820子功能返回的ARDS结构
; 每执行该中断一次便会得到一个ARDS结构的数据,放入缓冲区中
; 以便后续遍历
ards_buf times 244 db 0  ; ards_buf指向内存缓冲区地址,大小是244
                         ; 一个ARDS结构体是20字节, 可以容纳12个ARDS结构
ards_nr dw 0 ; 用于记录ARDS结构体的数量==>2字节

; 文件内偏移地址: 0x200(total_mem处的偏移)+(4+6+244+2)=0x200+0x100=0x300
; loader_start的实际地址在0xc00-:) 符合实模式低端1MB内存RAM可用区-:)
loader_start:
;-------------------------------------------------------
; bios 0x15中断 e820子功能 获取物理内存布局
;-------------------------------------------------------
; eax=0000E820h, edx=534D4150h ('SMAP')
    xor ebx, ebx         ; 自异或, 第一次调用时, ebx的值为0
                         ; 随后每次返回, CPU会重置ebx的值
    mov edx, 0x534d4150  ; ASCII code for 'SMAP', 固定签名:
                         ; BIOS将调用者正在请求的内存信息写入es:di寄存器
                         ; 所指向的ARDS缓冲区, 再用此签名校验其中的信息
    mov di, ards_buf     ; es:di指向ARDS结构缓冲区, es在mbr中已经赋值
.e820_mem_get_loop:      ; 循环获取每个ARDS返回的内存描述符结构
                         ; e820子功能返回ARDS是迭代式的
                         ; 直到EBX寄存器为0, 且CF位为0
    mov eax, 0x0000e820  ; bios0x15中断功能号
                         ; 执行int 15h后, eax的值会变成ascii('SMAP')
                         ; 每次调用中断前都要更新为子功能号
    mov ecx, 20          ; ARDS结构的字节大小: 4(字节)*5(表项)=20字节

    int 0x15             ; 调用bios0x15中断,子功能:e820h
    jc .e820_failed_so_try_e801  ; 判断CF、跳转: 0未出错, 1出错
    add di, cx           ; 增加di20字节指向新的ARDS结构位置
    inc word [ards_nr]   ; 增加ARDS结构体的数量
    cmp ebx, 0           ; 若ebx=0, CF!=1, 说明已是最后一个ARDS
    jnz .e820_mem_get_loop
; 在所有ards结构中
; 因为32位地址总线, 只要低32位就行了
; 找出(BaseAddLow+LengthLow)的最大值, 即最大物理内存容量
    mov cx, [ards_nr]    ; cx作为循环变量(ards的个数)
    mov ebx, ards_buf    ; ebx指向ards缓冲区的起始地址
    xor edx, edx         ; edx为最大内存容量, 初始置0
.find_max_mem_area:      ; 循环累加, 冒泡排序求最大
; 无需判断type是否为1, 最大的内存块一定是可以被使用的
    mov eax, [ebx]       ; BaseAddLow: 第1个表项:32位
    add eax, [ebx+8]     ; LengthLow: 第3个表项: 32位
    add ebx, 20          ; 指向缓冲区中下一个ARDS结构
    cmp edx, eax
; 冒泡排序, 找出最大内存容量放在edx寄存器中
    jge .next_ards
    mov edx, eax         ; edx<eax, 将当前的(BaseAddLow+LengthLow)给edx
                         ; edx即为最大物理内存容量
.next_ards:
    loop .find_max_mem_area
    jmp .mem_get_ok      ; 查找成功
;-------------------------------------------------------
; bios 0x15中断 e801子功能 获取物理内存布局 最大4G
;-------------------------------------------------------
; 返回后, ax,cx值一样, 以KB为单位; bx,dx值一样,以64KB为单位
; ax,cx寄存器中为低16MB(⚠ memory hole), bx,dx寄存器中为16MB~4GB
.e820_failed_so_try_e801:
    mov ax, 0xe801
    int 0x15
    jc .e801_failed_so_try88  ; carray flag: 0未出错, 1出错
; 先算出低15MB内存
; ax和cx是KB为单位->转换成byte为单位
    mov cx, 0x400 ;(1KB)
    mul cx ; 16位乘法, 乘数是AX寄存器. 积为32位, 高16位在DX寄存器, 低16位在AX寄存器中
           ; 计算返回的15MB以下内存容量AX*1KB
    and eax, 0x0000FFFF ; clear upper bits of eax
    shl edx, 16         ; edx左移16位: 空出edx低端16位
    or edx, eax         ; edx与eax做与运算得到完整的32位积=>15MB以下内存字节容量至edx
    add edx, 0x100000   ; ax只是15MB, 加1MB
    mov esi, edx        ; 先把低15MB的内存容量存入esi寄存器备份
; 再将16MB以上内存转化为以byte为单位
; 寄存器bx, dx中是以64KB为单位的内存数量
    xor eax, eax      ; eax清0
    mov ax, bx
    mov ecx, 0x10000  ; 0x10000 64KB
    mul ecx           ; 32位乘法
                      ; 默认的被乘数是eax, 积为64KB
                      ; 高32位存入edx, 低32位存入eax
    add esi, eax      ; 低32位足够了, 最大支持4GB
    mov edx, esi      ; edx为总内存大小: 低15+高16~4G+1MB(memory hole)
    jmp .mem_get_ok
;-------------------------------------------------------
; bios 0x15中断 0x88子功能 获取内存布局 只能获取64MB之内
;-------------------------------------------------------
.e801_failed_so_try88:
    mov ah, 0x88        ; 0x88号子功能
    int 0x15            ; 调用bios0x15中断
    jc .error_hlt       ; 出错了, 就挂起机器
    and eax, 0x0000FFFF

    ; 16位乘法: 被乘数是ax, 积为32位, 高16位在dx中, 低16位在ax中
    mov cx, 0x400 ;(1KB)
    mul cx
    shl edx, 16
    or edx, eax
    add edx, 0x100000   ; 0x88号子功能只会返回1MB以上的内存
                        ; 所以实际内存大小加上1MB
.mem_get_ok:
    mov [total_mem_bytes], edx  ; 以byte为单位的内存总量(✅ )

;-------------------------------------------------------
; bios 0x10中断 打印字符串
;-------------------------------------------------------
    ; mov sp, LOADER_BASE_ADDR
    ; mov bp, loadermsg
    ; mov cx, 15
    ; mov ax, 0x1301
    ; mov bx, 0x001f
    ; mov dx, 0x0C1F
    ; int 0x10
;-------------------------------------------------------
; 准备进入保护模式
; -> 1 打开A20
; -> 2 加载gdt
; -> 3 将cr0的pe位置为1
;-------------------------------------------------------
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

jmp dword SELECTOR_CODE:p_mode_start  ; 刷新CPU流水线
; 考虑到下面的部分指令取指和译码可能是在16位下完成的, 所以刷新流水线

.error_hlt:
    hlt  ; 出错则挂起

[bits 32]
p_mode_start:
    mov ax, SELECTOR_DATA  ; ds, es, ss被初始化为相同的选择子, 指向相同的数据段
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, LOADER_STACK_TOP
    mov ax, SELECTOR_VIDEO
    mov gs, ax
;-------------------------------------------------------
; 加载内核
; -> 把内核从硬盘取出放到内存缓冲区
;-------------------------------------------------------
    mov eax, KERNEL_START_SECTOR  ; kernel.bin所在的扇区号
    mov ebx, KERNEL_BIN_BASE_ADDR ; 从磁盘读出后, 写入到该内存地址
    mov ecx, 200                  ; 读入的扇区数

    call rd_disk_m_32             ; 把内核从磁盘中读取到ebx指定的内存地址
;-------------------------------------------------------
; 开启分页机制
; -> 初始化页目录表和页表
; -> 载入页目录表的物理地址到cr3
; -> 开启cr0的PG位为1
;-------------------------------------------------------
    ; 创建页目录及页表并初始化页内存分布
    call setup_page
    ; 准备重载GDT内相关描述符到内核地址空间(3GB以上)
    ; -- 显存段
    ; -- 栈段
    sgdt [gdt_ptr] ; 先将GDT放回到gdt_ptr指向的内存
    mov ebx, [gdt_ptr + 2] ; ebx是GDT的起始地址
    ; ebx+0x18(3*8byte)+4(段基址在段描述符中高4字节)=显存段在内存中的地址
    ; 和3GB(以上)起始地址: 0xc0000000做或运算, 得出重载后的显存段地址
    ; mov eax, 0xc0000000
    ; or eax, PG_US_U | PG_RW_W | PG_P
    or dword [ebx + 0x18 + 4], 0xc0000000
    ; or dword [ebx + 0x18 + 4], eax
    ; 得出重载后GDT基址
    add dword [gdt_ptr + 2], 0xc0000000
    ; add dword [gdt_ptr + 2], eax
    ; 栈指针也指向内核基地址
    add esp, 0xc0000000
    ; add esp, eax

    ; cr3加载页目录物理地址
    mov eax, PAGE_DIR_TABLE_POS
    ; mov eax, 0x100000
    mov cr3, eax  ; 控制寄存器和通用寄存器之间可以相互传递数据

    ; 打开cr0的pg位
    mov eax, cr0
    or eax, 0x80000000
    ;mov eax, 0xe0000011
    mov cr0, eax

    ; 开启分页后, 用gdt新的地址重新加载
    lgdt [gdt_ptr]

    jmp SELECTOR_CODE:enter_kernel  ; 利用jmp跳转指令, 强制刷新流水线, 更新gdt
                                    ; SELECTOR_CODE代码段(DPL)RPL0=>CPU CPL也为0, 当前CPU处于0特权级

enter_kernel:
    call kernel_init       ; 初始化内核映像
    mov esp, 0xc009f000    ; 初始化栈顶到0xc009f000
    jmp KERNEL_ENTRY_POINT ; jmp进入内核

    jmp $
;-------------------------------------------------------
; rd_disk_m_32: 硬盘内核文件读取
; -> eax: kernel.bin所在的扇区号
; -> ebx: 写入到的内存地址
; -> ecx: 读入的扇区数(可以设置为循环变量)
;-------------------------------------------------------
rd_disk_m_32:
    mov esi, eax    ; 备份eax
    mov di, cx      ; 备份cx
; 读写硬盘
; -> step1: 设置要读取的扇区数
    mov dx, 0x1f2   ; 0x1f2: primary通道, sector count寄存器
    mov al, cl      ; cl: the least significant 8 bits of the cx
    out dx, al      ; 读取的扇区数
    mov eax, esi    ; 恢复eax
; -> step2: 将lba地址存入0x1f3~0x1f6
    ;lbalow 7~0
    mov dx, 0x1f3
    out dx, al
    ;lbamid 15~8
    mov cl, 8
    shr eax, cl
    mov dx, 0x1f4
    out dx, al      ; al: the least significant 8 bits of the ax
    ;lbahigh 23~16
    shr eax, cl
    mov dx, 0x1f5
    out dx, al      ; 0
    ;lba device
    shr eax, cl
    and al, 0x0f
    or al, 0xe0     ; 11100000
    mov dx, 0x1f6
    out dx, al      ; device寄存器的低4位(存储lba24~27), 第4位0主盘;第6位1lba;第5和7位固定为1
; -> step3: 向0x1f7端口写入读命令
    mov dx, 0x1f7   ; 初始化状态寄存器
    mov al, 0x20    ; 0010 0000: DRDY: 设备就绪等待指令
    out dx, al      ; 写操作, command寄存器, 硬盘开始工作
; -> step4: 检测硬盘状态
    .not_ready:
        nop
        in al, dx
        and al, 0x88
        cmp al, 0x08
        jnz .not_ready
; -> step5: 从0x1f0端口读数据
        mov ax, di
        mov dx, 256
        mul dx
        mov cx, ax
        mov dx, 0x1f0
    .go_on_read:
        in ax, dx
        mov [ebx], ax
        add ebx, 2  ; ebx 操作数, 寻址方式反转, 防止读取的扇区数过大, 地址回卷, 破坏已经读写的内存
    loop .go_on_read
    ret
;-------------------------------------------------------
; 将kernel.bin中的segment拷贝到指定的地址
; -> 内核映像
;-------------------------------------------------------
kernel_init:
; eax, ebx, ecx, edx清零
    xor eax, eax
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx

    mov dx, [KERNEL_BIN_BASE_ADDR + 42]  ; dx: e_phentrysize: program header的大小
    mov ebx, [KERNEL_BIN_BASE_ADDR + 28]
    add ebx, KERNEL_BIN_BASE_ADDR        ; ebx: e_phoff: 第一个program header在内存中的物理地址
    mov cx, [KERNEL_BIN_BASE_ADDR + 44]  ; cx: e_phnum: program header的数目, 作为外层loop循环的循环计数
.each_segment:
    cmp byte [ebx + 0], PT_NULL
    je .PTNULL    ; 如果为1, 进入PTNULL, 否则继续
; 函数 mem_cpy(dst, src, size)
; -> 将参数压入栈中, 函数返回地址会自动压栈, 栈底实则为空
    push dword [ebx + 16]         ; p_filesz, 将第三个形参压入栈中
    mov eax, [ebx + 4]            ; p_offset
    add eax, KERNEL_BIN_BASE_ADDR ; eax为该段的物理地址
    push eax                      ; eax为该段源地址, 压入第二个参数
    push dword [ebx + 8]          ; p_vaddr: 压入第一个参数->目的地址

    call mem_cpy                  ; 调用mem_cpy函数
    add esp, 12                   ; 栈是向下扩展的, add esp, 将3个形参出栈, 垃圾回收(就是以后被覆盖orz...)
.PTNULL:
    add ebx, edx  ; 跳过空段, 拷贝下一个segment
    loop .each_segment  ; 因为如果没有空段, 那就是连续, 一次each_segment调用mem_cpy就可以拷贝完
    ret
;-------------------------------------------------------
; mem_cpy函数: 从源地址拷贝连续字节到目标地址
; -> movs[bwd] + rep + c[s]ld 三剑客...➖
;-------------------------------------------------------
mem_cpy:
    cld          ; clean direction, 设置eflags寄存器方向标志位DF为0, 自动增加esi, edi寄存器指向内存地址的值
    push ebp     ; 备份ebp, 外围"作用域"需要使用ebp
    mov ebp, esp ; ebp指向当前栈顶
    push ecx     ; 备份ecp, 外围
    ; 栈是向下扩展的
    mov edi, [ebp + 8]  ; 第一个形参: dst
    mov esi, [ebp + 12] ; 第二个形参: src
    mov ecx, [ebp + 16] ; 第三个形参: size (作为rep重复计数)
    rep movsb    ; movsb 一次从`DS:[E]SI`拷贝1个字节到`ES:[E]DI`
    ; 恢复环境
    pop ecx
    pop ebp
    ret
;-------------------------------------------------------
; 创建页目录表及页表
;-------------------------------------------------------
setup_page:
; 先把页目录占用的空间逐字节清0
; -> 页目录表包含1K个页目录表项, 每个表项是4字节32位
; -> 所以页目录表大小是4K
    mov ecx, 4096  ; 4kb(4096字节): 循环界限(CX寄存器)
    mov esi, 0
.clear_page_dir:
; 依据字节偏移量, 从页目录表基址开始逐字节清0
    mov byte [PAGE_DIR_TABLE_POS + esi], 0
    inc esi
    loop .clear_page_dir
; 开始创建页目录项(PDE)
.create_pde: ; 创建P(age)D(irectory)E(ntry)
    mov eax, PAGE_DIR_TABLE_POS
    add eax, 0x1000 ; 基址+偏移地址(4KB)->第一个页表起始物理地址(紧凑)
    mov ebx, eax    ; ebx作为第一个页表基址, 用于.create_pte
    or eax, PG_US_U | PG_RW_W | PG_P ; (0,1,2,3)+可写+位于内存中, 低3位用来控制内存页属性
; 在页目录表中的第1个目录项写入第1个页表的位置(0x101000)及属性
; 把loader的虚拟地址空间映射到低端1MB内存保证loader的正常运行
    mov [PAGE_DIR_TABLE_POS + 0x0], eax
; 一个页表占用4字节, 0xc00表示第(12*16*16/4=768)个页表占用的目录项
; 内核虚拟空间3GB以上(0xc0000000)映射到低端1MB内存0x101000处
    mov [PAGE_DIR_TABLE_POS + 0xc00], eax
; 现在页目录表第0项和第768项指向的都是同一个页表0x101000->物理内存(低端1MB)<-内核
; 0xc00000000~0xffffffff 1G属于内核
; 0x000000000~0xbfffffff 3G属于用户进程
    sub eax, 0x1000
; 页目录表的大小是4096byte->4kb, 4092是最后一个页目录表项的起始地址
    mov [PAGE_DIR_TABLE_POS + 4092], eax ; 使最后一个目录项指向页目录表自己的地址
; 创建页表项PTE(通过循环创建)
; loader, mbr, kernel都是运行在低端1MB内存之内的
; 所以目前只需要初始化低端1MB内存的页表(1MB)
    mov ecx, 256  ; 1M低端内存/4k=256, 一个物理页标准大小4K
    mov esi, 0
    mov edx, PG_US_U | PG_RW_W | PG_P
.create_pte:
    mov [ebx + esi*4], edx ; ebx已经被赋值为0x101000即第一个页表的地址
    add edx, 4096 ; 4KB按照一个物理页的字节数递归, 从而初始化整个页表
    inc esi
    loop .create_pte
; 创建内核其他页表的PDE
    mov eax, PAGE_DIR_TABLE_POS
    add eax, 0x2000 ; 页目录表的1KB+第0项1KB
    or eax, PG_US_U | PG_RW_W | PG_P
    mov ebx, PAGE_DIR_TABLE_POS
    mov ecx, 254
    mov esi, 769 ; 768+
.create_kernel_pde:
    mov [ebx+esi*4], eax
    inc esi
    add eax, 0x1000 ; 4KB增长循环初始化
    loop .create_kernel_pde
    ret
