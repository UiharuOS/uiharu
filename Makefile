# Makefile for UiharuOS

CC = gcc
OUT = build
Ttext = -Ttext $(KERNEL_ADDR)
CFLAGS = $(INCLUDE) -m32 -fno-builtin
INCLUDE = -Ilib/kernel/ -Ilib/ -Ikernel/
LIBCFLAGS = -I $(LIBINCLUDE) -m32
KERNEL_ADDR = 0xc0001500

all: kernel.bin

kernel.bin: kernel_main                \
			lib_kernel_print           \
			lib_kernel_print_string    \
			lib_kernel_print_int       \
			kernel_init                \
			kernel_interrupt           \
			kernel_kernel
	ld -m elf_i386 $(Ttext) -e main -o \
			${OUT}/kernel.bin          \
			${OUT}/main.o              \
			${OUT}/print.o             \
			${OUT}/print_string.o      \
			${OUT}/print_int.o         \
			${OUT}/init.o              \
			${OUT}/interrupt.o         \
			${OUT}/kernel.o

kernel_main: kernel/main.c
	$(CC) $(CFLAGS) -c -o ${OUT}/main.o kernel/main.c
lib_kernel_print: lib/kernel/print.S
	nasm -f elf32 -o ${OUT}/print.o lib/kernel/print.S
lib_kernel_print_string: lib/kernel/print_string.c
	$(CC) $(CFLAGS) -c -o ${OUT}/print_string.o lib/kernel/print_string.c
lib_kernel_print_int: lib/kernel/print_int.c
	$(CC) $(CFLAGS) -c -o ${OUT}/print_int.o lib/kernel/print_int.c
kernel_init: kernel/init.c
	$(CC) $(CFLAGS) -c -o ${OUT}/init.o kernel/init.c
kernel_interrupt: kernel/interrupt.c
	$(CC) $(CFLAGS) -c -o ${OUT}/interrupt.o kernel/interrupt.c
kernel_kernel: kernel/kernel.S
	nasm -f elf32 -o ${OUT}/kernel.o kernel/kernel.S

build:
	mkdir ${OUT}

clean:
	rm -rf ${OUT}
