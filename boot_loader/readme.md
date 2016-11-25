# UiharuOS::boot_loader

## 功能
boot_loader包含2个部分:**boot**和**loader**(似乎是废话...似乎是废话似乎是废话...似乎是废话似乎是废话似乎是废话...(不递归下去了...不递归下去了似乎是废话) 请无视以上废话:) <br/>
boot负责```bios加载mbr,mbr从硬盘中把loader加载入内存```, 工作在```CPU16位实模式```下 <br/>
loader负责开启保护模式、开启内存分页为加载内核做准备

## 特性
bios(basic input output system)🎬 <br/>

+ 计算机起始指令地址: 0xf000:0xfff0
+ --> 0xffff0: jmp 0xe05b
+ --> bios代码, 执行bios
    - POST
    - INT 19h -> mbr

<hr/>
mbr主引导记录📝  <br/>

+ mbr在0盘0道1扇区(CHS表示法)512字节
+ mbr最后2位0xaa55魔数(x86小端模式), 由bios检测识别
+ mbr被bios加载到[0x7c00](http://www.ruanyifeng.com/blog/2015/09/0x7c00.html)
+ mbr读写硬盘, 把loader加载到0x900
    + mbr在0x7c00
    + 实模式低端1MB内存```0x7e00~0x9fbff```约608KB是可用区域, 放loader和kernel足够了:)

<hr/>
loader内核加载器💻  <br/>

+ 为了加载内核, 需要进入保护模式, 以及开启内存分页
    + 进入保护模式
        + 打开A20地址线(没有了地址回卷)
        + 加载构造好的GDT
            - bootloader阶段只需用到```代码段、数据段和显存段```
            - 构造这3个段描述符即可
        + 将cr0的pe位置为1
    + 进入保护模式以后, 段基址存放```选择子```, 可以对内存段的访问进行细节的控制
    + 防止用户进程破坏内核
+ 为了更方便的内存控制(与硬盘交换等..)需要开启分页
    + ```核心思想把内核放到虚拟地址空间3GB以上(0xc0000000)内存中->映射到物理内存低端1MB```
    + 开启分页:
        + 创建页目录及页表, 初始化页内存分布
        + 准备重载GDT
            - 不希望普通用户调用显卡, 所以显存段重载到虚拟地址空间3GB以上
        + 将页目录表物理地址加载到cr3寄存器
        + 打开cr0的pg位
        + 重载GDT

## bootloader内存布局
![bootloader内存布局](https://cloud.githubusercontent.com/assets/10671733/20513472/00ac3f46-b0c1-11e6-96c8-a3302e6e6c83.png)

内核映像在虚拟地址空间```0xc0001500-->0x1500```
<hr/>
