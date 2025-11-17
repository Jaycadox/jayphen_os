CC := clang
SYSTEM_CC := clang
LD := lld
ASM := nasm

CFLAGS_GENERIC := -nostdlib -static -fno-builtin -ffreestanding -fno-stack-protector -std=c11 -Wall
CFLAGS := $(CFLAGS_GENERIC) -target x86_64-unknown-windows -Iedk2/MdePkg/Include -Iedk2/MdePkg/Include/X64 -I/usr/include
LDFLAGS := -flavor link -subsystem:efi_application -entry:EFIMain
ASMFLAGS := -f win64

default: all

uefiboot.o: bootloader/uefiboot.c
	$(CC) $(CFLAGS) -c $< -o obj/$@

sysv_elf_compat.o: bootloader/sysv_elf_compat.asm
	$(ASM) $(ASMFLAGS) $< -o obj/$@

test_shim: elf_loader/test_shim.c elf_loader/elfloader.c
	$(CC) $< -o $@

SYSTEM_PROGRAMS := system/HigherOrLower.c system/StartKernel.c
BINARIES := $(patsubst system/%.c,%,$(SYSTEM_PROGRAMS))
$(BINARIES): %: system/%.c
	$(SYSTEM_CC) $(CFLAGS_GENERIC) -Wl,--no-dynamic-linker -Wl,-z,norelro -Wl,-static -o root/$@.elf $<

bootx64.efi: uefiboot.o sysv_elf_compat.o $(BINARIES)
	$(LD) $(LDFLAGS) obj/uefiboot.o obj/sysv_elf_compat.o -out:root/EFI/boot/$@

.PHONY: qemu

qemu: bootx64.efi
	qemu-system-x86_64 -machine q35 --enable-kvm -cpu host -drive if=pflash,format=raw,file=./OVMF.4m.fd -drive format=raw,file=fat:rw:root -net none -m 1G


all: bootx64.efi
