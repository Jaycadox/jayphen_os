#pragma once

#include <__stdarg_va_list.h>
#include <stdbool.h>
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

// To make the compiler happy
void *memset(void *dst, int c, usize n) {
    uchar *d = dst;
    while (n--)
        *d++ = (uchar) c;
    return dst;
}

void *MemSet(void *dst, usize c, usize n) {
    uchar *d = dst;
    while (n--)
        *d++ = (uchar) c;
    return dst;
}

void *MemMove(void *dst, const void *src, usize n) {
    unsigned char       *d = dst;
    const unsigned char *s = src;

    if (d < s) {
        for (size_t i = 0; i < n; i++)
            d[i] = s[i];
    } else if (d > s) {
        for (size_t i = n; i > 0; i--)
            d[i - 1] = s[i - 1];
    }

    return dst;
}

size_t StrLen(const char *s) {
    size_t n = 0;
    while (s[n] != '\0')
        n++;
    return n;
}

typedef int64_t (*SysInvoke)(i64 call, usize userdata);
static SysInvoke SYSINVOKE;

void UEFIPrintLine(char *Text) {
    SYSINVOKE(0, (usize) Text);
}

void UEFIPrint(char *Text) {
    SYSINVOKE(1, (usize) Text);
}

void UEFIPrintNumber(uint64_t Number) {
    SYSINVOKE(2, (usize) Number);
}

i64 UEFIReadLine(char *LineBuffer) {
    return SYSINVOKE(3, (usize) LineBuffer);
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

struct FrameBufferPixel {
    u8 Blue;
    u8 Green;
    u8 Red;
    u8 Reserved;
};
struct FrameBufferInfo {
    struct FrameBufferPixel *Base;
    usize                    Size;

    u32 Width, Height;
    // u32 PixelsPerScanline;
};

static struct FrameBufferInfo *gFrameBuffer = NULL;

#include "kernel/Terminal.c"

struct FrameBufferInfo *GetFrameBufferInfo(void) {
    return (struct FrameBufferInfo *) (usize) SYSINVOKE(6, 0);
}

void ExitUEFIMode(void) {
    SYSINVOKE(5, 0);
}

void UEFIPrintf(const char *Format, ...) __attribute__((format(printf, 1, 2)));
void UEFIPrintf(const char *Format, ...) {
    char    PrintBuffer[512] = {0};
    va_list args;
    va_start(args, Format);

    stbsp_vsnprintf(PrintBuffer, sizeof(PrintBuffer), Format, args);
    va_end(args);
    UEFIPrint(PrintBuffer);
}

void UEFIPrintLinef(const char *Format, ...) __attribute__((format(printf, 1, 2)));
void UEFIPrintLinef(const char *Format, ...) {
    char    PrintBuffer[512] = {0};
    va_list args;
    va_start(args, Format);

    stbsp_vsnprintf(PrintBuffer, sizeof(PrintBuffer), Format, args);
    va_end(args);
    UEFIPrintLine(PrintBuffer);
}

// Kernel section

i64 StringToInt64(char *String) {
    if (!String)
        return 0;

    i64 result = 0;
    i32 sign   = 1;

    while (*String == ' ' || *String == '\t' || *String == '\n' || *String == '\r' || *String == '\v' || *String == '\f') {
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

void Panic(const char *Format, ...) __attribute__((format(printf, 1, 2)));
void Panic(const char *Format, ...) {
    ClearTerminal(0x55, 0x0, 0x0);
    PrintLinef("!!! KERNEL PANIC !!!");

    va_list args;
    va_start(args, Format);
    PrintLinefv(Format, args);
    va_end(args);
    for (;;)
        ;
}

i32 main();

i32 _start(void *userdata) {
    SYSINVOKE = userdata;
    return main();
}
