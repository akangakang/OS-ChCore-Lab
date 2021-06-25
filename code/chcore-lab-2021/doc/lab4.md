# 实验4: 多核处理

## 练习1

> 阅读boot/start.S中的汇编代码_start。比较实验三和本实验之间_start部分代码的差异（可使用git diff lab3命令） 。说明_start如何选定主CPU，并阻塞其他副CPU 的执行。

**选定主CPU：**

```assembly
mrs	x8, mpidr_el1
and	x8, x8,	#0xFF
cbz	x8, primar
```

先读取寄存器`mpidr_el1`的值，并取后`8`位

如果为`0`，则为主CPU，跳转到`primary`标签执行代码

如果不为`0`，说明是副CPU

若是首个CPU则进行层级切换，准备函数栈和异常向量，进行初始化

**阻塞其他副CPU 的执行：**

1. 如果是副CPU，首先等待`bss`段清空

   ```assembly
   wait_for_bss_clear:
   	adr	x0, clear_bss_flag
   	ldr	x1, [x0]
   	cmp     x1, #0
   	bne	wait_for_bss_clear
   
   	/* Turn to el1 from other exception levels. */
   	bl 	arm64_elX_to_el1
   
   	/* Prepare stack pointer and jump to C. */
   	mov	x1, #0x1000
   	mul	x1, x8, x1
   	adr 	x0, boot_cpu_stack
   	add	x0, x0, x1
   	add	x0, x0, #0x1000
           mov	sp, x0
   ```

   **循环检查**`clear_bss_flag`的值，如果不为`0`则说明还未清空`bss`段，继续循环等待。

   如果成功清空，则执行函数`arm64_elX_to_el1`，转为`EL1`异常级别

   然后准备栈指针

2. 等待主CPU显式激活副CPU

   ```assembly
   wait_until_smp_enabled:
   	/* CPU ID should be stored in x8 from the first line */
   	mov	x1, #8
   	mul	x2, x8, x1
   	ldr	x1, =secondary_boot_flag
   	add	x1, x1, x2
   	ldr	x3, [x1]
   	cbz	x3, wait_until_smp_enabled
   
   	/* Set CPU id */
   	mov	x0, x8
   	bl 	secondary_init_
   ```

   **循环检查**`secondary_boot_flag`，直到不为`0`则跳出循环，否则一直循环等待

   设置`CPU id`后执行函数`	secondary_init_`激活副CPU

3. 进行层级切换，准备函数栈和异常向量，进行初始化

相反，`lab3`中

```assembly
secondary_hang:
	bl secondary_hang
```

一直循环副CPU，不会被激活



## 练习3

> 熟悉启动副CPU 的控制流程（同主CPU 类似），并回答以下问题：初始化时，主CPU 同时激活所有副CPU 而不是依次激活每个副CPU 的设计是否正确？换言之，并行启动副CPU 是否会导致并发问题？提示：检查每个CPU 核心是否共享相同的内核堆栈以及控制流中的每个函数调用是否会导致数据竞争。

正确

每个CPU核心不共享内核栈。

```c
char kernel_stack[PLAT_CPU_NUM][KERNEL_STACK_SIZE];
```

各个副CPU仅共享`clear_bss_flag`，但仅读取其中的数据，不会导致数据竞争。



## 练习6

> 为了保护寄存器上下文， 在el0_syscall调用lock_kernel()时，在栈上保存了寄存器的值。然而， 在exception_return中调用unlock_kernel()时，却不需要将寄存器的值保存到栈中，试分析其原因。

因为执行完`unlock_kernel`后，将执行`exception_exit`。

`exception_exit`函数将从内核栈中恢复用户态的寄存器，并回到用户态。这也意为着，执行`unlock_kernel`之后，不再需要使用`Caller_save`的寄存器，所以不再需要保护。



## 练习8

> 如果异常是从内核态捕获的，CPU 核心不会在kernel/exception/irq.c的handle_irq中获得了大内核锁。但是，有一种特殊情况，即如果空闲线程（以内核态运行）中捕获了错误，则CPU 核心还应该获取大内核锁。否则，内核可能会被永远阻塞。请思考一下原因。

空闲线程运行在内核态，但是没有持有大内核锁。

在`handl_irq`函数结束时，将会调用`eret_to_thread(switch_context())`，释放大内核锁。如果空闲进程不拿大内核锁，但是释放一次大内核锁，`unlock`过程会将`lock->owner++`。

考虑以下情况：

此时`lock->owner = n+1`且`lock->next=n` 。如果有两个空闲线程没有拿锁就放锁，`lock->owner`则会为`n+2`。如果此时有一个CPU核上的非空闲的线程A想要拿锁，进行“取号”，`lock->next=n+1`，因为此时`lock->owner = n+2 >n+1`，所以线程A将永远阻塞，永远拿不到大内核锁。





