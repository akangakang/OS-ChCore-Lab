#define BEGIN_FUNC(_name)	\
	.global _name;		\
    .type   _name, %function;	\
_name:

#define END_FUNC(_name)		\
	.size _name, .-_name

BEGIN_FUNC(START)
	mov x29, #0
	mov x30, #0
	mov x0, sp
	and sp, x0, #-16
	b _start_c
END_FUNC(START)
