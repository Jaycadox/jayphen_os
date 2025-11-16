#pragma once

#define TERMINAL_FONT_RAW 8
#define TERMINAL_FONT_SCALE 2
#define TERMINAL_FONT_PX (TERMINAL_FONT_RAW * TERMINAL_FONT_SCALE)
#include "../elf_libc.c"
#include "font8x8.h"

struct Terminal {
    u64 Position;
};

static struct Terminal gTerminal = {0};

void TerminalPutChar(char Char) {
    u64 MaximumCharactersPerRow = (gFrameBuffer->Width / TERMINAL_FONT_PX);
    u64 MaximumRows             = (gFrameBuffer->Height / TERMINAL_FONT_PX);
    u64 Capacity                = MaximumCharactersPerRow * MaximumRows;
    u64 CharacterYIndex         = gTerminal.Position / MaximumCharactersPerRow;
    u64 CharacterXIndex         = gTerminal.Position % MaximumCharactersPerRow;

    u64 NextPosition = gTerminal.Position + 1;
    if (Char == '\n') {
        NextPosition = (CharacterYIndex + 1) * MaximumCharactersPerRow;
    }
    if (Char == '\t') {
        TerminalPutChar(' ');
        TerminalPutChar(' ');
        TerminalPutChar(' ');
        TerminalPutChar(' ');
        return;
    }

    if (NextPosition >= Capacity) {
        usize NumPixels = ((MaximumRows - 1) * TERMINAL_FONT_PX) * gFrameBuffer->Width;
        usize NumBytes  = NumPixels * sizeof(struct FrameBufferPixel);

        struct FrameBufferPixel *Source =
            gFrameBuffer->Base + (TERMINAL_FONT_PX * gFrameBuffer->Width);
        struct FrameBufferPixel *Dest = gFrameBuffer->Base;

        MemMove(Dest, Source, NumBytes);

        usize                    NumPixelsToBlank = TERMINAL_FONT_PX * gFrameBuffer->Width;
        struct FrameBufferPixel *BlankPixelsSource =
            gFrameBuffer->Base + ((MaximumRows - 1) * TERMINAL_FONT_PX * gFrameBuffer->Width);
        MemSet(BlankPixelsSource, 0x0, NumPixelsToBlank * sizeof(struct FrameBufferPixel));
        CharacterYIndex    = MaximumRows - 1;
        CharacterXIndex    = 0;
        gTerminal.Position = CharacterYIndex * MaximumCharactersPerRow;
        NextPosition       = (Char == '\n') ? gTerminal.Position : gTerminal.Position + 1;
    }

    u64 CharacterYPos = CharacterYIndex * TERMINAL_FONT_PX;
    u64 CharacterXPos = CharacterXIndex * TERMINAL_FONT_PX;

    char *PixelData = font8x8_basic[Char];
    for (u64 YOffset = 0; YOffset < TERMINAL_FONT_PX; ++YOffset) {
        char RowData = PixelData[YOffset / TERMINAL_FONT_SCALE];
        for (u64 XOffset = 0; XOffset < TERMINAL_FONT_PX; ++XOffset) {
            u64  Y       = CharacterYPos + YOffset;
            u64  X       = CharacterXPos + XOffset;
            bool Enabled = RowData & (1 << (XOffset / TERMINAL_FONT_SCALE));

            if (Enabled) {
                gFrameBuffer->Base[Y * gFrameBuffer->Width + X].Red      = 0xFF;
                gFrameBuffer->Base[Y * gFrameBuffer->Width + X].Green    = 0xFF;
                gFrameBuffer->Base[Y * gFrameBuffer->Width + X].Blue     = 0xFF;
                gFrameBuffer->Base[Y * gFrameBuffer->Width + X].Reserved = 0xFF;
            }
        }
    }
    gTerminal.Position = NextPosition;
}

void TerminalPutString(char *String) {
    usize Length = StrLen(String);
    for (usize i = 0; i < Length; ++i) {
        TerminalPutChar(String[i]);
    }
}

void Print(char *String) {
    TerminalPutString(String);
}

void PrintLine(char *String) {
    Print(String);
    TerminalPutChar('\n');
}

void Printf(const char *Format, ...) {
    char    PrintBuffer[512] = {0};
    va_list args;
    va_start(args, Format);

    stbsp_vsnprintf(PrintBuffer, sizeof(PrintBuffer), Format, args);
    va_end(args);
    Print(PrintBuffer);
}

void PrintLinef(const char *Format, ...) {
    char    PrintBuffer[512] = {0};
    va_list args;
    va_start(args, Format);

    stbsp_vsnprintf(PrintBuffer, sizeof(PrintBuffer), Format, args);
    va_end(args);
    PrintLine(PrintBuffer);
}
