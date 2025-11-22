#pragma once
#include "../elf_libc.c"

void EnableProgrammableIntervalTimer(u32 FrequencyHz) {
    OutByte(0x20, 0x11);
    OutByte(0xA0, 0x11);

    OutByte(0x21, 0x20);
    OutByte(0xA1, 0x28);

    OutByte(0x21, 4);
    OutByte(0xA1, 2);

    OutByte(0x21, 1);
    OutByte(0xA1, 1);

    OutByte(0x21, 0xFE);
    OutByte(0xA1, 0xFF);

    u32 Divisor = 1193180 / FrequencyHz;

    OutByte(0x43, 0x36);

    OutByte(0x40, Divisor & 0xFF);
    OutByte(0x40, (Divisor >> 8) & 0xFF);
}
