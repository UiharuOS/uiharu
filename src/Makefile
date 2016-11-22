# Makefile for UiharuOS

kernel.bin: kernel_main.o lib_kernel_print.o
	ld -m elf_i386 -Ttext 0xc0001500 -e main -o kernel.bin kernel/main.o lib/kernel/print.o

kernel_main.o: kernel/main.c
	gcc -I lib/kernel -c -o kernel/main.o kernel/main.c

lib_kernel_print.o: lib/kernel/print.S
	nasm -f elf32 -o lib/kernel/print.o lib/kernel/print.S
