#define binary_include(file, name) 	\
	.align 4;                       \
	.globl binary_##name##_start;	\
	binary_##name##_start:; 		\
	.incbin #file;					\
	binary_##name##_end:; 			\
	.globl binary_##name##_size; 			\
	binary_##name##_size:; 			\
	.quad binary_##name##_end - binary_##name##_start

.section .rodata
binary_include(../user/build/ramdisk.cpio, cpio_bin)
