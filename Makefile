# Makefile for UiharuOS

CC = gcc
OUT = build
KERNEL_ADDR = 0xc0001500
Ttext = -Ttext $(KERNEL_ADDR)
LIBCFLAGS = -I $(LIBINCLUDE) -m32
CFLAGS = $(INCLUDE) -m32 -fno-builtin
INCLUDE = -Ilib/ -Ikernel/ -Ikernel/lib/ -Ikernel/device/ -Ikernel/thread/ -Iacademy-city/

all: kernel.bin

kernel.bin: kernel_main                \
            kernel_lib_print           \
            kernel_lib_print_string    \
            kernel_lib_print_int       \
            kernel_lib_bitmap          \
            kernel_lib_dlist           \
            kernel_init                \
            kernel_interrupt           \
            kernel_interrupt_switch    \
            kernel_kernel              \
            kernel_debug               \
            kernel_device_timer        \
            kernel_device_set_cursor   \
            kernel_memory              \
            kernel_thread              \
            kernel_thread_switch       \
            lib_string
	ld -m elf_i386 $(Ttext) -e main -o \
            ${OUT}/kernel.bin          \
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
            ${OUT}/string.o            \
            ${OUT}/bitmap.o            \
            ${OUT}/memory.o            \
            ${OUT}/thread.o            \
            ${OUT}/dlist.o             \
            ${OUT}/switch.o            \
            ${OUT}/set_cursor.o

kernel_main: kernel/main.c
	$(CC) $(CFLAGS) -c -o ${OUT}/main.o kernel/main.c
kernel_lib_print: kernel/lib/print.S
	nasm -f elf32 -o ${OUT}/print.o kernel/lib/print.S
kernel_kernel: kernel/kernel.S
	nasm -f elf32 -o ${OUT}/kernel.o kernel/kernel.S
kernel_thread_switch: kernel/thread/switch.S
	nasm -f elf32 -o ${OUT}/switch.o kernel/thread/switch.S
kernel_device_set_cursor: kernel/device/set_cursor.S
	nasm -f elf32 -o ${OUT}/set_cursor.o kernel/device/set_cursor.S
kernel_lib_print_string: kernel/lib/print_string.c
	$(CC) $(CFLAGS) -c -o ${OUT}/print_string.o kernel/lib/print_string.c
kernel_lib_print_int: kernel/lib/print_int.c
	$(CC) $(CFLAGS) -c -o ${OUT}/print_int.o kernel/lib/print_int.c
kernel_lib_bitmap: kernel/lib/bitmap.c
	$(CC) $(CFLAGS) -c -o ${OUT}/bitmap.o kernel/lib/bitmap.c
kernel_lib_dlist: kernel/lib/dlist.c
	$(CC) $(CFLAGS) -c -o ${OUT}/dlist.o kernel/lib/dlist.c
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
kernel_thread: kernel/thread/thread.c
	$(CC) $(CFLAGS) -c -o ${OUT}/thread.o kernel/thread/thread.c
kernel_device_timer: kernel/device/timer.c
	$(CC) $(CFLAGS) -c -o ${OUT}/timer.o kernel/device/timer.c

lib_string: lib/string.c
	$(CC) $(CFLAGS) -c -o ${OUT}/string.o lib/string.c

build:
	mkdir ${OUT}

clean:
	rm -rf ${OUT}
