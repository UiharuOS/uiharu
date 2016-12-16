;-------------	 loader和kernel   ----------
LOADER_BASE_ADDR equ 0x900            ; loader被读入到内存中的地址
LOADER_START_SECTOR equ 0x2           ; loader在硬盘的起始扇区(lba)
KERNEL_BIN_BASE_ADDR equ 0x70000      ; 内核在内存中的地址
KERNEL_IMAGE_BASE_ADDR  equ   0x1500  ; 内核映像在内存中的地址
KERNEL_START_SECTOR equ 0x9           ; 内核在硬盘的起始扇区(lba)
KERNEL_ENTRY_POINT equ 0xc0001500     ; 内核起始虚拟地址, 0xc0001500 -> 0x1500

PAGE_DIR_TABLE_POS equ 0x100000  ; 页目录表放到低端1MB内存以上

;------------------   gdt描述符属性  --------------------
DESC_G_4K         equ	 1_00000000000000000000000b ; G位:段界限粒度1字节或4KB
DESC_D_32         equ	 1_0000000000000000000000b  ; D位:D/B字段分别指定代码段和栈段段内偏移地址和操作数的大小
DESC_L	          equ	 0_000000000000000000000b	; 64位代码标记，此处标记为0便可。
DESC_AVL          equ	 0_00000000000000000000b	; cpu不用此位，暂置为0
                                                    ; 段界限用20个[2进制位]表示, 段界限是单位量, 段的大小要么是1MB, 要么是4GB
DESC_LIMIT_CODE2  equ    1111_0000000000000000b     ; 代码段描述符段界限的16~19位
DESC_LIMIT_DATA2  equ    1111_0000000000000000b     ; 数据段描述符段界限的16~19位
DESC_LIMIT_VIDEO2 equ    0000_000000000000000b      ; 显存段描述符段界限的16~19位

DESC_P	          equ    1_000000000000000b         ; 段存在于内存中(1)或被置换到硬盘上(0)
DESC_DPL_0        equ    00_0000000000000b          ; 段描述符的特权级 0
DESC_DPL_1        equ	 01_0000000000000b          ; 段描述符的特权级 1
DESC_DPL_2        equ	 10_0000000000000b          ; 段描述符的特权级 2
DESC_DPL_3        equ	 11_0000000000000b          ; 段描述符的特权级 3
DESC_S_CODE       equ	 1_000000000000b            ; S指定该内存段的类别 1->代码段\__ 数据段 
DESC_S_DATA       equ	 DESC_S_CODE                ;                      ->数据段/         \_ CPU眼里做的划分
DESC_S_sys        equ	 0_000000000000b            ;                     0->系统段(门结构)  /
DESC_TYPE_CODE    equ    1000_00000000b	            ; x=1,c=0,r=0,a=0 代码段是可执行的,非依从的,不可读的,已访问位a清0.
DESC_TYPE_DATA    equ    0010_00000000b	            ; x=0,e=0,w=1,a=0 数据段是不可执行的,向上扩展的,可写的,已访问位a清0.

;---------- 利用上面的属性构造段描述符高32位 -----------
; 工作在平坦模式下段基址为0, 特权级全部设为0
DESC_CODE_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + DESC_P + DESC_DPL_0 + DESC_S_CODE + DESC_TYPE_CODE + 0x00
DESC_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00
DESC_VIDEO_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x0b

;--------------   选择子属性  ---------------
RPL0     equ  00b
RPL1     equ  01b
RPL2     equ  10b
RPL3     equ  11b
TI_GDT	 equ  000b
TI_LDT	 equ  100b

;----------------   页表相关属性    --------------
PG_P     equ  1b
PG_RW_R	 equ  00b
PG_RW_W	 equ  10b
PG_US_S	 equ  000b
PG_US_U	 equ  100b

;----------  elf program header相关属性 ----------
PT_NULL  equ  0  ; 空段类型
