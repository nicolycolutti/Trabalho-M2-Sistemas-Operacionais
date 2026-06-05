CROSS = riscv64-unknown-elf-
CFLAGS = -march=rv64gc -mabi=lp64 \
         -mcmodel=medany \
         -msmall-data-limit=0 \
         -ffreestanding \
         -nostdlib -nostartfiles \
         -fno-stack-protector \
         -Wall -Iinclude

OBJS = start.o trap_entry.o context.o \
       main.o task.o scheduler.o uart.o string.o memory.o

all: kernel.elf

kernel.elf: $(OBJS)
	$(CROSS)ld -T linker.ld $(OBJS) -o kernel.elf

start.o: boot/start.S
	$(CROSS)gcc $(CFLAGS) -c boot/start.S -o start.o

trap_entry.o: boot/trap_entry.S
	$(CROSS)gcc $(CFLAGS) -c boot/trap_entry.S -o trap_entry.o

context.o: kernel/context.S
	$(CROSS)gcc $(CFLAGS) -c kernel/context.S -o context.o

main.o: kernel/main.c
	$(CROSS)gcc $(CFLAGS) -c kernel/main.c -o main.o

task.o: kernel/task.c
	$(CROSS)gcc $(CFLAGS) -c kernel/task.c -o task.o

scheduler.o: kernel/scheduler.c
	$(CROSS)gcc $(CFLAGS) -c kernel/scheduler.c -o scheduler.o

uart.o: kernel/uart.c
	$(CROSS)gcc $(CFLAGS) -c kernel/uart.c -o uart.o

string.o: kernel/string.c
	$(CROSS)gcc $(CFLAGS) -c kernel/string.c -o string.o

memory.o: kernel/memory.c
	$(CROSS)gcc $(CFLAGS) -c kernel/memory.c -o memory.o

clean:
	rm -f *.o kernel.elf

run: kernel.elf
	qemu-system-riscv64 \
		-machine virt \
		-m 128M \
		-nographic \
		-bios default \
		-kernel kernel.elf