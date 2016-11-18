; 配置文件
; ----------------- loader & kernel -----------------
LOADER_BASE_ADDR equ 0x900      ; 加载器被加载至内存中的地址
LOADER_START_SECTOR equ 0x2     ; 加载器起始扇区(硬盘的第二个扇区)
KERNEL_BIN_BASE_ADDR equ 0x70000
KERNEL_IMAGE_BASE_ADDR equ 0x1500
KERNEL_START_SECTOR equ 0x9
PAGE_DIR_TABLE_POS equ 0x100000 ; 页目录表起始地址:低端1MB内存的第一个字节(偏移地址)

; ----------------- gdt descriptor ------------------
DESC_G_4K equ 1_00000000000000000000000b ; 段描述符G位设定4K精度, 访问范围0~4G ⚡️ 23
DESC_D_32 equ 1_0000000000000000000b     ; 段描述符D位设定操作数32位           ⚡️ 22
DESC_L    equ 0_000000000000000000000b   ; 段描述符L位设定, 置为0表示32位CPU   ⚡️ 21
DESC_AVL  equ 0_000000000000000000000b   ; CPU不用此位, 暂时置为0              ⚡️ 20
DESC_LIMIT_CODE2  equ 1111_0000000000000000b ;                                 ⚡️ 19~16
DESC_LIMIT_DATA2  equ DESC_LIMIT_CODE2       ;                                 ⚡️ 19~16
DESC_LIMIT_VIDEO2 equ  0000_000000000000000b ;                                 ⚡️ 19~16
DESC_P    equ      1_000000000000000b ; CPU检查段在内存中                      ⚡️ 15
DESC_DPL_0 equ 00_0000000000000b ; 0特权级                                     ⚡️ 14
DESC_DPL_1 equ 01_0000000000000b ; 1特权级                                     ⚡️ 14~13
DESC_DPL_2 equ 10_0000000000000b ; 2特权级                                     ⚡️ 14~13
DESC_DPL_3 equ 11_0000000000000b ; 3特权级                                     ⚡️ 14~13
DESC_S_CODE     equ 1_000000000000b ; 非系统段~代码段                          ⚡️ 12
DESC_S_DATA     equ DESC_S_CODE     ; 非系统段~数据段                          ⚡️ 12
DESC_S_sys      equ 0_000000000000b ; 系统段                                   ⚡️ 12
DESC_TYPE_CODE  equ 1000_00000000b
; 代码段 可执行 非一致性 不可读 已访问位a清0                                   ⚡️ 11~8
DESC_TYPE_DATA  equ 0010_00000000b
; 数据段 不可执行 向上扩展 可写 已访问位a清0                                   ⚡️ 11~8
; 代码段的高4字节
DESC_CODE_HIGH4 equ (0x00 << 24)+DESC_G_4K+DESC_D_32+DESC_L+DESC_AVL+DESC_LIMIT_CODE2+DESC_P+DESC_DPL_0+DESC_S_CODE+DESC_TYPE_CODE+0x00;                      ⚡️ 24~31
DESC_DATA_HIGH4 equ (0x00 << 24)+DESC_G_4K+DESC_D_32+DESC_L+DESC_AVL+DESC_LIMIT_DATA2+DESC_P+DESC_DPL_0+DESC_S_DATA+DESC_TYPE_DATA+0x00;                      ⚡️ 24~31
DESC_VIDEO_HIGH4 equ (0x00 << 24)+DESC_G_4K+DESC_D_32+DESC_L+DESC_AVL+DESC_LIMIT_VIDEO2+DESC_P+DESC_DPL_0+DESC_S_DATA+DESC_TYPE_DATA+0x0b;                    ⚡️ 24~31

; ----------- selector -----------
RPL0 equ 00b
RPL1 equ 01b
RPL2 equ 10b
RPL3 equ 11b
TI_GDT equ 000b
TI_LDT equ 100b

; ---------- page table ----------
PG_P equ 1b        ; 存在位, 默认为1表示该页存在物理内存中
PG_RW_R equ 00b    ; 读写位, 只读
PG_RW_W equ 10b    ; 读写位, 可写
PG_US_S equ 000b   ; 特权级: 0, 1, 2    可访问
PG_US_U equ 100b   ; 特权级: 0, 1, 2, 3 可访问
