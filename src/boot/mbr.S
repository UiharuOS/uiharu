;UIHARU::MBR
;---------------------------------------------------------------------------
%include "boot.inc"
SECTION MBR vstart=0x7c00
  mov ax, cs
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov fs, ax
  mov sp, 0x7c00 ; mbr stack address
  mov ax, 0xb800 ; graphics port address
                 ; 0xb8000/16 = 0xb800 >> cpu real mode
  mov gs, ax 

;CLEAR SCREEN
;use bios INT 0x10
;---------------------------------------------------------------------------
  mov ax, 0x600  ; bios int function number
  mov bx, 0x700
  mov cx, 0      ; the upper left->(0, 0)->(0x0, 0x0)
  mov dx, 0x184f ; the lower right->(80, 25)->(79, 24)->(0x4f, 0x18)

  int 0x10

;PRINT BG-COLOR GREEN, FR-COLOR RED, FLASH STRING "UIHARU"
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

mov eax, LOADER_START_SECTOR    ; the lba address of start sector
mov bx, LOADER_BASE_ADDR        ; data write memory address
mov cx, 1                       ; the number of sector to read
call rd_disk_m_16               ; 

jmp LOADER_BASE_ADDR

;read n sectors in disk
;---------------------------------------------------------------------------
rd_disk_m_16:
    mov esi, eax
    mov di, cx

    mov dx, 0x1f2
    mov al, cl
    out dx, al

    mov eax, esi

    ;lba 7~0
    mov dx, 0x1f3
    out dx, al

    ;lba 15-8
    mov cl, 8
    shr eax, cl
    mov dx, 0x1f4
    out dx, al

    ;lba 23-16
    shr eax, cl
    mov dx, 0x1f5
    out dx, al

    shr eax, cl
    and al, 0x0f
    or al, 0xe0
    mov dx, 0x1f6
    out dx, al

    mov dx, 0x1f7
    mov al, 0x20
    out dx, al
    
  .not_ready:
    nop
    in al, dx
    and al, 0x88
    cmp al, 0x08
    jnz .not_ready

    mov ax, di
    mov dx, 256
    mul dx
    mov cx, ax

    mov dx, 0x1f0
  .go_on_read:
    in ax, dx
    mov [bx], ax
    add bx, 2
    loop .go_on_read
    ret

times (512-2)-($-$$) db 0
db 0x55, 0xaa
