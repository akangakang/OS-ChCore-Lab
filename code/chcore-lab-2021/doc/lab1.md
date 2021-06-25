# lab1

## Exercise 1

浏览《ARM 指令集参考指南》的A1、A3 和D 部分，以熟悉ARM ISA。请做好阅读笔记，如果之前学习x86-64 的汇编，请写下与x86-ww64 相比的一些差异

------

| 区别 | AArch64 | x86-64 |
| :--: | :--: | :--: |
| 特权级 | EL0 /EL1 /EL2 /EL3 | Non-root / Root |
| 指令集 | RISC | CISC |
| 通用寄存器 | 31个 | 16 个 |
| 栈寄存器 | 4个 | 1个(切换特权级rsp压栈) |
| 系统状态寄存器 | PSTATE | EFLAGS |



## Exercise 2

启动带调试的QEMU，使用GDB 的where命令来跟踪入口（第一个函数）及bootloader 的地址。

------

```
0x0000000000080000 in ?? ()
(gdb) where
#0  0x0000000000080000 in _start ()
```
入口为 `_start()`
地址为 `0x0000000000080000`



## Exercise 3

### Exercise 3.1
结合readelf -S build/kernel.img读取符号表与练习2 中的GDB 调试信息，请找出请找出build/kernel.image入口定义在哪个文件中。

------

```bash
os@ubuntu:~/Desktop/os/chcore-lab-2021$ readelf -S build/kernel.img
There are 9 section headers, starting at offset 0x20cd8:

Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] init              PROGBITS         0000000000080000  00010000
       000000000000b5b0  0000000000000008 WAX       0     0     4096
  [ 2] .text             PROGBITS         ffffff000008c000  0001c000
       00000000000011dc  0000000000000000  AX       0     0     8
  [ 3] .rodata           PROGBITS         ffffff0000090000  00020000
       00000000000000f8  0000000000000001 AMS       0     0     8
  [ 4] .bss              NOBITS           ffffff0000090100  000200f8
       0000000000008000  0000000000000000  WA       0     0     16
  [ 5] .comment          PROGBITS         0000000000000000  000200f8
       0000000000000032  0000000000000001  MS       0     0     1
  [ 6] .symtab           SYMTAB           0000000000000000  00020130
       0000000000000858  0000000000000018           7    46     8
  [ 7] .strtab           STRTAB           0000000000000000  00020988
       000000000000030f  0000000000000000           0     0     1
  [ 8] .shstrtab         STRTAB           0000000000000000  00020c97
       000000000000003c  0000000000000000           0     0     1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  p (processor specific)
```
由`ELF`文件知,`.init`地址为`0x0000000000080000`

结合Exercise 2知,入口为`_start()`,在`boot/start.S`文件中



### Exercise 3.2
继续借助单步调试追踪程序的执行过程，思考一个问题：目前本实验中支持的内核是单核版本的内核，然而在Raspi3 上电后，所有处理器会同时启动。结合boot/start.S中的启动代码，并说明挂起其他处理器的控制流。

------

```assembly
BEGIN_FUNC(_start)
	mrs	x8, mpidr_el1       /* mpdir_el1 记录了当前P的cpuid, mrs 对其进行读取并存入x8*/
	and	x8, x8,	#0xFF       /* 保留x8的低8位 */
	cbz	x8, primary         /* 如果x8为0, 则为首个CPU, 跳转到 primary */

secondary_hang:             /* 如果不是首个CPU则 hang */
	bl secondary_hang

primary:
    /* 如果是首个CPU */
	/* 调用函数 arm64_elX_to_el1 将异常级别切换到EL1 */
	bl 	arm64_elX_to_el1

	/* 准备函数栈和异常向量 */
	adr 	x0, boot_cpu_stack
	add 	x0, x0, #0x1000
	mov 	sp, x0

	bl 	init_c

	/* Should never be here */
	b	.
END_FUNC(_start)
```
通过读取寄存器 mpidr_el1 的值判断是否为首个CPU。

若是首个CPU则进行层级切换，准备函数栈和异常向量，进行初始化。否则hang住。



## Exercise 4
查看build/kernel.img的objdump信息。比较每一个段中的VMA 和LMA 是否相同，为什么？在VMA 和LMA 不同的情况下，内核是如何将该段的地址从LMA 变为VMA？提示：从每一个段的加载和运行情况进行分析

------

```bash
os@ubuntu:~/Desktop/os/chcore-lab-2021$ objdump -h build/kernel.img 

build/kernel.img:     file format elf64-little

Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  0 init          0000b5b0  0000000000080000  0000000000080000  00010000  2**12
                  CONTENTS, ALLOC, LOAD, CODE
  1 .text         000011dc  ffffff000008c000  000000000008c000  0001c000  2**3
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  2 .rodata       000000f8  ffffff0000090000  0000000000090000  00020000  2**3
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  3 .bss          00008000  ffffff0000090100  0000000000090100  000200f8  2**4
                  ALLOC
  4 .comment      00000032  0000000000000000  0000000000000000  000200f8  2**0
                  CONTENTS, READONLY
```
`init`，`.comment`的LMA和VMA相同

`.text`，`.rodata`，`.bss`的LMA和VMA相差了`0xffffff00000`

因为`init`存放的是Bootloader的代码，kernel的代码放在其他段中。Bootloader开始运行时处于实模式，不支持虚拟内存，无法访问高地址。

在初始化的过程中，`init_boot_pt()`会初始化页表，`el1_mmu_activate()`开启MMU，将kernel代码映射到低地址段和高地址段。所以进入内核后VMA变成了高地址，寻址的时候也是用的VMA。



## Exercise 5

见代码



## Exercise 6

内核栈初始化（即初始化SP 和FP）的代码位于哪个函数？内核栈在内存中位于哪里？内核如何为栈保留空间？

------

```assembly
BEGIN_FUNC(start_kernel)
    /* 
     * Code in bootloader specified only the primary 
     * cpu with MPIDR = 0 can be boot here. So we directly
     * set the TPIDR_EL1 to 0, which represent the logical
     * cpuid in the kernel 
     */
    mov     x3, #0
    msr     TPIDR_EL1, x3

    ldr     x2, =kernel_stack
    add     x2, x2, KERNEL_STACK_SIZE
    mov     sp, x2
    bl      main
END_FUNC(start_kernel)
```
内核栈的初始化在`start_kernel`函数中  (`kernel/head.S`)

在`kernel/main.c`里可以找到`kernel_stack`的定义
```c
char kernel_stack[PLAT_CPU_NUM][KERNEL_STACK_SIZE];
```
在`kernel/common/vars.h`中可以找到对`KERNEL_STACK_SIZE`的定义

```c
#define KERNEL_STACK_SIZE       (8192)
```

内核栈初始化让SP指向`kernel_stack[0]`的第8192字节处。因为栈是由高地址向低地址增长，所以第8192字节前的空间即为留给内核栈的空间。

```
(gdb) info variables
All defined variables:

Non-debugging symbols:
0x00000000000900f8  _bss_start
0x00000000000900f8  _edata
0x0000000000098100  _bss_end
0x00000000000a0000  img_end
0xffffff0000090100  kernel_stack
```

可以看到`kernel_stack`的内存地址是`0xffffff0000090100`，是未初始化的全局变量，在内存中位于`.bss`段



## Exercise 7

每个stack_test递归嵌套级别将多少个64位值压入堆栈，这些值是什么含义？

2. 在stack_test打断点
```
(gdb) b stack_test
Breakpoint 1 at 0xffffff000008c03c
```
3. 函数开始时FP与SP相同
```
(gdb) info register sp
sp             0xffffff00000920f0       0xffffff00000920f0 <kernel_stack+8176>
(gdb) info register fp
fp             0xffffff00000920f0       0xffffff00000920f0 <kernel_stack+8176>
```
4. 查看第1次运行到该函数时的栈  
(注:此时 sp=0xffffff00000920f0)
```
(gdb) x/32g $x29
...
0xffffff00000920d0 <kernel_stack+8144>: 0x0000000000000000      0x0000000000000061
0xffffff00000920e0 <kernel_stack+8160>: 0x0000000000080000      0x00000000ffffffc0
0xffffff00000920f0 <kernel_stack+8176>: 0x0000000000000000      0xffffff000008c018
```
5. 查看第2次运行到此函数时的栈  
(注:此时 sp=0xffffff00000920d0)
```
(gdb) info register sp
fp             0xffffff00000920d0       0xffffff00000920d0 <kernel_stack+8144>
(gdb) x/32g $x29
...
0xffffff00000920d0 <kernel_stack+8144>: 0xffffff0000092000      0xffffff000008c0d4
0xffffff00000920e0 <kernel_stack+8160>: 0x0000000000000000      0x00000000ffffffc0
0xffffff00000920f0 <kernel_stack+8176>: 0x0000000000000000      0xffffff000008c018
```
6. 对比两次可发现栈增长了32字节,也就是每个stack_test递归嵌套级别将**4个64位值压入堆栈**.分析栈可发现
```
+--------------------------------+  <-------+  
|                                |
|   local (0x00000000ffffffc0)   |
|                                |
+--------------------------------+
|                                |
|   param (0x0000000000000000)   |
|                                |
+--------------------------------+
|                                |
|  ret adr (0xffffff000008c0d4)  |
|                                |
+--------------------------------+
|                                |
|   x29 (0xffffff0000092000)     |
|                                |
+--------------------------------+  <-------+  

```
分别为**临时变量**,**参数**,**返回地址**,**保存FP**



## Exercise 8
为了复用这些寄存器，这些寄存器中原来的值是如何被存在栈中的？请使用示意图表示，回溯函数所需的信息（如SP、FP、LR、参数、部分寄存器值等）在栈中具体保存的位置在哪？

-----

```
                   +------+ +--------------+
                   |        |  local data  |
                   |        +--------------+
                   |        |     arg1     |
                   |        +--------------+
                   |        |     ....     |
                   |        +--------------+
high               |        |     argN     |
                   |        +--------------+
                   |        |      LR      |
 +                 |        +--------------+
 |                 |        |    last FP   |
 |                 +------+ +--------------+ <---+ FP'
 |                 |        |  local data  |
 |                 |        +--------------+
 |                 |        |     arg1     |
 |                 |        +--------------+
 |                 |        |     ....     |
 v                 |        +--------------+
                   |        |     argN     |
                   |        +--------------+
low                |        |      LR      |
                   |        +--------------+
                   |        | last FP (FP')|
                   +------+ +--------------+ <----+ FP
                   |   当   |  local data  |
                   |   前   +--------------+
                   |   函   |     arg1     |
                   |   数   +--------------+
                   |        |     ....     |
                   +------+ +--------------+ <----+ SP

```



## Exercise 9

使用与示例相同的格式， 在kernel/monitor.c中实现stack_backtrace。为了忽略编译器优化等级的影响，只需要考虑stack_test的情况，我们已经强制了这个函数编译优化等级。挑战：请思考，如果考虑更多情况（例如，多个参数）时，应当如何进行回溯操作？

-----

见代码