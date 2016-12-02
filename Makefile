# Makefile for UiharuOS

INCLUDE = lib/kernel/ lib/userland/
LIBINCLUDE = lib/
CC = gcc
CFLAGS = -I $(INCLUDE) -m32
LIBCFLAGS = -I $(LIBINCLUDE) -m32
KERNEL_ADDR = 0xc0001500
Ttext = -Ttext $(KERNEL_ADDR)

all: kernel.bin

kernel.bin: kernel_main.o lib_kernel_print.o lib_kernel_print_string.o lib_kernel_print_int.o
	ld -m elf_i386 $(Ttext) -e main -o kernel.bin kernel/main.o lib/kernel/print.o lib/kernel/print_string.o lib/kernel/print_int.o

kernel_main.o: kernel/main.c
	$(CC) $(CFLAGS) -c -o kernel/main.o kernel/main.c

lib_kernel_print.o: lib/kernel/print.S
	nasm -f elf32 -o lib/kernel/print.o lib/kernel/print.S
lib_kernel_print_string.o: lib/kernel/print_string.c
	$(CC) $(CFLAGS) -c -o lib/kernel/print_string.o lib/kernel/print_string.c
lib_kernel_print_int.o: lib/kernel/print_int.c
	$(CC) $(CFLAGS) -c -o lib/kernel/print_int.o lib/kernel/print_int.c

clean:
	rm **/*.o
