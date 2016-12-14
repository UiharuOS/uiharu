# Makefile for UiharuOS

CC = gcc
OUT = build
Ttext = -Ttext $(KERNEL_ADDR)
CFLAGS = $(INCLUDE) -m32 -fno-builtin
INCLUDE = -Ilib/kernel/ -Ilib/ -Ikernel/ -Idevice/
LIBCFLAGS = -I $(LIBINCLUDE) -m32
KERNEL_ADDR = 0xc0001500

all: kernel.bin

kernel.bin: kernel_main                \
            lib_string                 \
            lib_kernel_print           \
            lib_kernel_print_string    \
            lib_kernel_print_int       \
            lib_kernel_bitmap          \
            kernel_init                \
            kernel_interrupt           \
            kernel_interrupt_switch    \
            kernel_kernel              \
            kernel_debug               \
            device_timer
	ld -m elf_i386 $(Ttext) -e main -o \
            ${OUT}/kernel.bin          \
            ${OUT}/string.o            \
            ${OUT}/main.o              \
            ${OUT}/print.o             \
            ${OUT}/print_string.o      \
            ${OUT}/print_int.o         \
            ${OUT}/init.o              \
            ${OUT}/interrupt.o         \
            ${OUT}/interrupt_switch.o  \
            ${OUT}/kernel.o            \
            ${OUT}/debug.o             \
            ${OUT}/timer.o             \
            ${OUT}/bitmap.o

kernel_main: kernel/main.c
	$(CC) $(CFLAGS) -c -o ${OUT}/main.o kernel/main.c

lib_string: lib/string.c
	$(CC) $(CFLAGS) -c -o ${OUT}/string.o lib/string.c

lib_kernel_print: lib/kernel/print.S
	nasm -f elf32 -o ${OUT}/print.o lib/kernel/print.S
lib_kernel_print_string: lib/kernel/print_string.c
	$(CC) $(CFLAGS) -c -o ${OUT}/print_string.o lib/kernel/print_string.c
lib_kernel_print_int: lib/kernel/print_int.c
	$(CC) $(CFLAGS) -c -o ${OUT}/print_int.o lib/kernel/print_int.c
lib_kernel_bitmap: lib/kernel/bitmap.c
	$(CC) $(CFLAGS) -c -o ${OUT}/bitmap.o lib/kernel/bitmap.c

kernel_kernel: kernel/kernel.S
	nasm -f elf32 -o ${OUT}/kernel.o kernel/kernel.S
kernel_init: kernel/init.c
	$(CC) $(CFLAGS) -c -o ${OUT}/init.o kernel/init.c
kernel_interrupt: kernel/interrupt.c
	$(CC) $(CFLAGS) -c -o ${OUT}/interrupt.o kernel/interrupt.c
kernel_interrupt_switch: kernel/interrupt_switch.c
	$(CC) $(CFLAGS) -c -o ${OUT}/interrupt_switch.o kernel/interrupt_switch.c
kernel_debug: kernel/debug.c
	$(CC) $(CFLAGS) -c -o ${OUT}/debug.o kernel/debug.c
	
device_timer: device/timer.c
	$(CC) $(CFLAGS) -c -o ${OUT}/timer.o device/timer.c

build:
	mkdir ${OUT}

clean:
	rm -rf ${OUT}
