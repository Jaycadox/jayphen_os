#pragma once

#include <stddef.h>
#include <stdint.h>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define usize size_t
#define uchar unsigned char

#define i8 int8_t
#define i16 int16_t
#define i32 int32_t
#define i64 int64_t

void *memcpy(void *dst, const void *src, usize n) {
    uchar       *d = dst;
    const uchar *s = src;
    while (n--)
        *d++ = *s++;
    return dst;
}

void *memset(void *dst, usize c, usize n) {
    uchar *d = dst;
    while (n--)
        *d++ = (uchar) c;
    return dst;
}

typedef int64_t (*SysInvoke)(i64 call, usize userdata);
static SysInvoke SYSINVOKE;

void PrintLine(char *Text) {
    SYSINVOKE(0, (usize) Text);
}

void Print(char *Text) {
    SYSINVOKE(1, (usize) Text);
}

void PrintNumber(uint64_t Number) {
    SYSINVOKE(2, (usize) Number);
}

i64 ReadLine(char *LineBuffer) {
    return SYSINVOKE(3, (usize) LineBuffer);
}

i64 StringToInt64(char *String) {
    if (!String)
        return 0;

    i64 result = 0;
    i32 sign   = 1;

    while (*String == ' ' || *String == '\t' || *String == '\n' || *String == '\r' ||
           *String == '\v' || *String == '\f') {
        String++;
    }

    if (*String == '-') {
        sign = -1;
        String++;
    } else if (*String == '+') {
        String++;
    }

    while (*String >= '0' && *String <= '9') {
        i32 digit = *String - '0';
        result    = result * 10 + digit;
        String++;
    }

    return result * sign;
}

u64 RandomInt64(void) {
    u64   value;
    uchar ok;

    for (i32 i = 0; i < 10; i++) {
        __asm__ volatile("rdrand %0; setc %1" : "=r"(value), "=qm"(ok) : : "cc");

        if (ok)
            return value;
    }

    __asm__ volatile("rdtsc" : "=A"(value));
    return value;
}

// See bootloader/uefiboot_libc.c, as these definitions need to match
struct MemoryRegion {
    u64 Start;
    u64 NumPages;
};

#define MAX_REGIONS 32
struct MemoryLayout {
    usize NumConventionalRegions;
    usize NumMemoryMappedIORegions;
    usize NumPortMappedIORegions;
    usize NumACPIRegions;

    struct MemoryRegion ConventionalRegions[MAX_REGIONS];
    struct MemoryRegion MemoryMappedIORegions[MAX_REGIONS];
    struct MemoryRegion PortMappedIORegions[MAX_REGIONS];
    struct MemoryRegion ACPIRegions[MAX_REGIONS];
};

struct MemoryLayout *GetMemoryLayout(void) {
    return (struct MemoryLayout *) (usize) SYSINVOKE(4, 0);
}

#define STB_SPRINTF_IMPLEMENTATION
#include "./vendor/stb_sprintf.h"
#include <stdarg.h>

void Printf(const char *Format, ...) __attribute__((format(printf, 1, 2)));
void Printf(const char *Format, ...) {
    char    PrintBuffer[512] = {0};
    va_list args;
    va_start(args, Format);

    stbsp_vsnprintf(PrintBuffer, sizeof(PrintBuffer), Format, args);
    va_end(args);
    Print(PrintBuffer);
}

void PrintLinef(const char *Format, ...) __attribute__((format(printf, 1, 2)));
void PrintLinef(const char *Format, ...) {
    char    PrintBuffer[512] = {0};
    va_list args;
    va_start(args, Format);

    stbsp_vsnprintf(PrintBuffer, sizeof(PrintBuffer), Format, args);
    va_end(args);
    PrintLine(PrintBuffer);
}

i32 main();

i32 _start(void *userdata) {
    SYSINVOKE = userdata;
    return main();
}
