



# 实验2: 内存管理

## 问题 1

请简单解释，在哪个文件或代码段中指定了ChCore 物理内存布局。你可以从两个方面回答这个问题: 编译阶段和运行时阶段。

------

**编译阶段** : 由`scripts/linker-aarch64.lds.in`确定`img_start`到`img_end`的布局  

* `img_start` = `0x80000`
* `img_end` = `ALIGN(SZ_64K)` - `KERNEL_VADDR`

**运行时阶段** : 由`kernel/mm/mm.c`确定`img_end`以上的布局 

`img_end`以上空闲的物理内存由物理页分配器管理

* 文件确定了`img_end`以上的布局为

  ||` metadata` (`npages` * `sizeof`(`struct page`))  || `start_vaddr` ... (`npages` * `PAGE_SIZE`) ||

  分为元数据范围和页面范围

* 在`mm_init`函数中确定各个数据范围的气势和结束地址，使用`init_buddy`进行初始化



## 问题 2
AArch64 采用了两个页表基地址寄存器，相较于x86-64 架构中只有一个页表基地址寄存器，这样的好处是什么？请从性能与安全两个角度做简要的回答。

------

**安全**: 
 * 较好的隔离，将内核和用户态的页表隔离。用户态的错误不会导致内核的崩溃

**性能**：
* 系统调用不需要切换页表，可以避免TLB刷新的开销，性能较好 



## 问题 3

1. 请问在页表条目中填写的下一级页表的地址是物理地址还是虚拟地址?
* 物理地址
2. 在`ChCore` 中检索当前页表条目的时候，使用的页表基地址是虚拟地址还是物理地址？
* 物理地址



## 问题 4

### 问题 4.1
如果我们有4G 物理内存，管理内存需要多少空间开销? 这个开销是如何降低的?

------

64位地址空间，一个页表项占`8 bytes`
$$
4GB = 2 ^{32} \\
size = \frac{2^{32}}{4K}* 8 bytes = 8 MB
$$


通过多级页表降低开销，多级页表允许在整个页表结构中出现空洞

### 问题 4.2
总结一下`x86-64` 和`AArch64` 地址翻译机制的区别，`AArch64 MMU`架构设计的优点是什么? 

**区别**：

* **页表基地址寄存器** ：
    * `AArch64`应用程序和操作系统使用不同的页表，硬件提供了`TTBR0_EL1`,`TTBR1_EL1`两个不同的页表基地址寄存器
    * `x86-64`只提供一个页表基地址寄存器`CR3`，操作系统不使用单独的页表，把操作系统映射到应用程序页表中的高地址部分
* **属性位:**
    * `AArch64`有：`UXN`,`PXN`,`AF`,`SH`,`AP`,`NS`,`Indx`属性
    * `x86-64`有：`XD`,`G`,`D`,`A`,`CD`,`WT`,`U/S`,`R/W`,`P`属性
* **页表标签** ：
    * `AArch64` 使用`ASID`， `OS`为不同进程分配8/16 `ASID`，将`ASID`填写在`TTBR0_EL1`的高8/16位 
    * `x86-64`  使用`PCID` ， 存储在`CR3`的低12位
* **刷新其他核`TLB`** ：
    * `AArch64` : 可在local CPU 上刷新其他核`TLB`
    * `x86-64` : 发送`IPI`中断给某个核，通知它主动刷新
* **缺页异常** ：
    * `AArch64` : 缺页异常没有一个专门的异常号，触发8号同步异常，操作系统根据`ESR`信息判断是否缺页，错误地址在`FAR_EL1`
    * `x86-64 `: 出发14号异常，错误地址放在`CR2`寄存器中
* **最小页面大小** ：
    * `AArch64` ：支持多种最小页面大小，`TCR_EL1`可以配置3种：`4K`,`16K`,`64K`
    * `x86-64` : 只支持`4K`



**优点：**

1. `AArch64 MMU`的隔离性更好，因为内核和用户态使用不同页表
2. `AArch64 MMU`可以在local CPU 上刷新其他核`TLB`，不需要核间通信，更快
3. 支持多种最小页面大小，更加灵活




## 问题 5
在AArch64 MMU 架构中，使用了两个TTBR 寄存器，ChCore 使用一个TTBR 寄存器映射内核地址空间，另一个寄存器映射用户态的地址空间，那么是否还需要通过设置页表位的属性来隔离内核态和用户态的地址空间?

------

需要

因为用户态和内核态都可以访问这两个页表基地址寄存器


## 问题 6
### 问题 6.1
1. `ChCore` 为什么要使用块条目组织内核内存?
* 使用块组织内核内存可以减少`TLB`缓存项的使用，提高命中率
* 且减少页表级数，提升查询页表的效率。使内核运行更快。
2. 哪些虚拟地址空间在Boot 阶段必须映射，哪些虚拟地址空间可以在内核启动后延迟?
* 在boot阶段必须映射（KBASE-KBASE + 256M）段的虚拟地址空间，剩余可以延迟映射

   

### 问题 6.2

为什么用户程序不能读写内核内存? 保护内核内存的具体机制是什么?

------

因为用户程序是不可信的。如果用户程序可以读写内核内存，用户程序的bug或者恶意攻击，写了内核不该写的地方，可能会导致整个系统的崩溃。

**保护内核内存的机制**：

1. 描述符中的内存属性可以保护内核内存。比如`UXN`,`PXN`限制了可执行性，`AP`限制了访问权限等
2. 用户和内核使用两个不同的页表基地址寄存器。用户和内核使用不同的页表，确保了隔离性，更加安全

