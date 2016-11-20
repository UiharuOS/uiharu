# Uiharu OS

> neo1218's hobby OS

+ **32bit**✅ **x86**😄 +**posix**⚡️ 

## BootLoader
+ **homebrew**
    1. 0xf000:fff0 -> 0xe05b -> bios
    2. 0x7c00 -> mbr
    3. 0x9000 -> loader
        + 构造GDT
        + 开启32位保护模式
        + 开启内存分页
    4. kernel:) -> written in C
+ **grub**
    - 再说:)
