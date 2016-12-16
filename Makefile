# Makefile for UiharuOS

CC = gcc
OUT = build
Ttext = -Ttext $(KERNEL_ADDR)
CFLAGS = $(INCLUDE) -m32 -fno-builtin
INCLUDE = -Ilib/ -Ikernel/ -Ikernel/lib/ -Idevice/ -Iacademy-city/
LIBCFLAGS = -I $(LIBINCLUDE) -m32
KERNEL_ADDR = 0xc0001500

all: kernel.bin

kernel.bin: kernel_main                \
            kernel_lib_print           \
            kernel_lib_print_string    \
            kernel_lib_print_int       \
            kernel_lib_bitmap          \
            kernel_init                \
            kernel_interrupt           \
            kernel_interrupt_switch    \
            kernel_kernel              \
            kernel_debug               \
            lib_string                 \
            device_timer               \
			kernel_memory
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
            ${OUT}/bitmap.o            \
			${OUT}/memory.o

kernel_main: kernel/main.c
	$(CC) $(CFLAGS) -c -o ${OUT}/main.o kernel/main.c
kernel_lib_print: kernel/lib/print.S
	nasm -f elf32 -o ${OUT}/print.o kernel/lib/print.S
kernel_kernel: kernel/kernel.S
	nasm -f elf32 -o ${OUT}/kernel.o kernel/kernel.S
kernel_lib_print_string: kernel/lib/print_string.c
	$(CC) $(CFLAGS) -c -o ${OUT}/print_string.o kernel/lib/print_string.c
kernel_lib_print_int: kernel/lib/print_int.c
	$(CC) $(CFLAGS) -c -o ${OUT}/print_int.o kernel/lib/print_int.c
kernel_lib_bitmap: kernel/lib/bitmap.c
	$(CC) $(CFLAGS) -c -o ${OUT}/bitmap.o kernel/lib/bitmap.c
kernel_init: kernel/init.c
	$(CC) $(CFLAGS) -c -o ${OUT}/init.o kernel/init.c
kernel_interrupt: kernel/interrupt.c
	$(CC) $(CFLAGS) -c -o ${OUT}/interrupt.o kernel/interrupt.c
kernel_interrupt_switch: kernel/interrupt_switch.c
	$(CC) $(CFLAGS) -c -o ${OUT}/interrupt_switch.o kernel/interrupt_switch.c
kernel_debug: kernel/debug.c
	$(CC) $(CFLAGS) -c -o ${OUT}/debug.o kernel/debug.c
kernel_memory: kernel/memory.c
	$(CC) $(CFLAGS) -c -o ${OUT}/memory.o kernel/memory.c

lib_string: lib/string.c
	$(CC) $(CFLAGS) -c -o ${OUT}/string.o lib/string.c
	
device_timer: device/timer.c
	$(CC) $(CFLAGS) -c -o ${OUT}/timer.o device/timer.c

build:
	mkdir ${OUT}

clean:
	rm -rf ${OUT}
