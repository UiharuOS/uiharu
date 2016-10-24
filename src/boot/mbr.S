;MBR
;------------------------------------------------------------------------------
SECTION MBR vstart=0x7c00 ;tell compiler set start address as 0x7c00
  mov ax, cs
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov fs, ax
  mov sp, 0x7c00 ; mbr stack address
;------------------------------------------------------------------------------
;use bios INT 0x10 (bochx86->little-endian)
;------------------------------------------------------------------------------
  mov ax, 0x600
  mov bx, 0x700
  mov cx, 0	 ; (0, 0)
  mov dx, 0x184f ; (0x18, 0x4f)
; get cursor position
  mov ah, 3 ; n3 child function...
  mov bh, 0
  int 0x10
;------------------------------------------------------------------------------
; print string
;------------------------------------------------------------------------------
  mov ax, message
  mov bp, ax
  mov cx, 9
  mov ax, 0x1301
  mov bx, 0x2
  int 0x10
;------------------------------------------------------------------------------
; end
;------------------------------------------------------------------------------
  jmp $
  message db "neo1218:)"
  times 510-($-$$) db 0
  db 0x55, 0xaa
