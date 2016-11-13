# Uiharu OS

> neo1218's hobby OS

+ **32bit**
+ **x86**
+ **posix**

## BootLoader
+ **homebrew**

    0xf000:fff0->0xe05b->bios->(bs=512 seek=1 0xaa55)⚡️ mbr->(硬盘)loader ---> kernel
    \_______________________ 实模式 _______________________/\_ 打开A20Gate;加载GDT;置cr0 pe位为1 进入保护模式_/

+ **grub**
    - 再说:)

![uiharu](https://cloud.githubusercontent.com/assets/10671733/19648940/790d61d2-9a36-11e6-9456-afc791a1ded9.jpg)

## Change logs
### 161113⚡️ [uiharuOS0_0_0_0_2.iso](https://github.com/UiharuOS/uiharu/blob/master/disk_images/uiharuOS0_0_0_0_2.iso)

+ issue: can't find loader

### 161102⚡️ [uiharuOS0_0_0_0_1.iso](https://github.com/UiharuOS/uiharu/blob/master/disk_images/uiharuOS0_0_0_0_1.iso)
