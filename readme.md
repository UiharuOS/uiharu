# Uiharu OS

> neo1218's hobby OS

+ **32bit**✅ **x86**😄 **posix**⚡️ 

## [📝 BootLoader](https://github.com/UiharuOS/uiharu/tree/master/src/boot_loader)
主引导记录, 内核加载

## [❤️ Kernel](https://github.com/UiharuOS/uiharu/tree/master/src/kernel)
内核, written in c

## [🔧 Tools](https://github.com/UiharuOS/uiharu/tree/master/src/tools)
常用脚本

## [📜 Lib](https://github.com/UiharuOS/uiharu/tree/master/src/lib)
各种库文件(函数), 供kernel, userland使用

<hr/>

## [🍙 Bochs](https://github.com/UiharuOS/uiharu/tree/master/bochs)
Bochs虚拟机模拟环境配置, 可作为硬件环境参考

## [🎫 DiskImages](https://github.com/UiharuOS/uiharu/tree/master/disk_images)
某不科学的iso文件🐛 , 方便安装uiharu

<hr/>
## ⚡️ Build Kernel

    $ cd src
    $ make
    $ ./tools/write_kernel_2_disk.sh kernel.bin

build 32 bit elf binary file: ```src/kernel.bin```
