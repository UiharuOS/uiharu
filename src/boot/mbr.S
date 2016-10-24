;uiharu::MBR
;
;LOADER_BASE_ADDR equ 0xA000
;LOADER_START_SECTOR equ 0x2
;---------------------------------------------------------------------------
SECTION MBR vstart=0x7c00
  mov ax, cs
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov fs, ax
  mov sp, 0x7c00 ; mbr stack address
  mov ax, 0xb800 ; graphics port address
                 ; 0xb8000/16 = 0xb800 >>
  mov gs, ax 
;---------------------------------------------------------------------------
;clear screen
;use bios INT 0x10 (bochx86->little-endian)
;---------------------------------------------------------------------------
  mov ax, 0x600  ; bios int function number
  mov bx, 0x700
  mov cx, 0      ; the upper left->(0, 0)->(0x0, 0x0)
  mov dx, 0x184f ; the lower right->(80, 25)->(79, 24)->(0x4f, 0x18)
; get cursor position
;  mov ah, 3 ; n3 child function...
;  mov bh, 0
  int 0x10
;;------------------------------------------------------------------------------
;; print string
;;------------------------------------------------------------------------------
;  mov ax, message
;  mov bp, ax
;  mov cx, 9
;  mov ax, 0x1301
;  mov bx, 0x2
;  int 0x10
;;------------------------------------------------------------------------------
;; end
;;------------------------------------------------------------------------------
;  jmp $
;  message db "neo1218:)"
;  times 510-($-$$) db 0
;  db 0x55, 0xaa

;---------------------------------------------------------------------------
; print bg-color green, fr-color red, flash string "Uiharu"
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

mov byte [gs:0x10], 'u'
mov byte [gs:0x11], 0xA4


jmp $

times (512-2)-($-$$) db 0
db 0x55, 0xaa
