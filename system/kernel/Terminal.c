#pragma once

#define TERMINAL_FONT_SCALE 1
#define TERMINAL_FONT_RAW_X 8
#define TERMINAL_FONT_PX_X (TERMINAL_FONT_RAW_X * TERMINAL_FONT_SCALE)

#define TERMINAL_FONT_RAW_Y 16
#define TERMINAL_FONT_PX_Y (TERMINAL_FONT_RAW_Y * TERMINAL_FONT_SCALE)

#include "../elf_libc.c"
#include "font8x16.h"
#include "font8x8.h"

struct TerminalColour {
    u8 Red;
    u8 Green;
    u8 Blue;
};

struct Terminal {
    u64                   Position;
    struct TerminalColour TextColour;
    bool                  TextColourInit;
};

#define TERMINAL_DEFAULT_COLOUR ((struct TerminalColour) {.Red = 0xFF, .Green = 0xFF, .Blue = 0xFF})

#define TERMINAL_DEBUG_COLOUR ((struct TerminalColour) {.Red = 0xFF, .Green = 0xFF, .Blue = 0x00})

static struct Terminal gTerminal = {0};

void ClearTerminal(u8 Red, u8 Green, u8 Blue) {
    for (int x = 0; x < gFrameBuffer->Width; ++x) {
        for (int y = 0; y < gFrameBuffer->Height; ++y) {
            gFrameBuffer->Base[y * gFrameBuffer->Width + x].Blue  = Blue;
            gFrameBuffer->Base[y * gFrameBuffer->Width + x].Green = Green;
            gFrameBuffer->Base[y * gFrameBuffer->Width + x].Red   = Red;
        }
    }
    gTerminal.TextColour = TERMINAL_DEFAULT_COLOUR;
    gTerminal.Position   = 0;
}

void TerminalPutChar(char Char) {
    if (!gTerminal.TextColourInit) {
        gTerminal.TextColour     = TERMINAL_DEFAULT_COLOUR;
        gTerminal.TextColourInit = true;
    }

    u64 MaximumCharactersPerRow = (gFrameBuffer->Width / TERMINAL_FONT_PX_X);
    u64 MaximumRows             = (gFrameBuffer->Height / TERMINAL_FONT_PX_Y);
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
        usize NumPixels = ((MaximumRows - 1) * TERMINAL_FONT_PX_Y) * gFrameBuffer->Width;
        usize NumBytes  = NumPixels * sizeof(struct FrameBufferPixel);

        struct FrameBufferPixel *Source = gFrameBuffer->Base + (TERMINAL_FONT_PX_Y * gFrameBuffer->Width);
        struct FrameBufferPixel *Dest   = gFrameBuffer->Base;

        u32* DestPtr = (u32*)Dest;
        u32* SrcPtr = (u32*)Source;
        usize PixelCount = NumBytes / 4;

        for (usize i = 0; i < PixelCount; ++i) {
            DestPtr[i] = SrcPtr[i];
        }

        usize                    NumPixelsToBlank  = TERMINAL_FONT_PX_Y * gFrameBuffer->Width;
        struct FrameBufferPixel *BlankPixelsSource = gFrameBuffer->Base + ((MaximumRows - 1) * TERMINAL_FONT_PX_Y * gFrameBuffer->Width);
        MemSet(BlankPixelsSource, 0x0, NumPixelsToBlank * sizeof(struct FrameBufferPixel));
        CharacterYIndex    = MaximumRows - 1;
        CharacterXIndex    = 0;
        gTerminal.Position = CharacterYIndex * MaximumCharactersPerRow;
        NextPosition       = (Char == '\n') ? gTerminal.Position : gTerminal.Position + 1;
    }

    u64 CharacterYPos = CharacterYIndex * TERMINAL_FONT_PX_Y;
    u64 CharacterXPos = CharacterXIndex * TERMINAL_FONT_PX_X;
    if (Char != '\n' && Char != '\r' && Char != '\t') {
        char *PixelData = font8x16[(usize) Char];
        for (u64 YOffset = 0; YOffset < TERMINAL_FONT_PX_Y; ++YOffset) {
            char RowData = PixelData[YOffset / TERMINAL_FONT_SCALE];
            for (u64 XOffset = 0; XOffset < TERMINAL_FONT_PX_X; ++XOffset) {
                u64  Y       = CharacterYPos + YOffset;
                u64  X       = CharacterXPos + XOffset;
                bool Enabled = RowData & (1 << ((TERMINAL_FONT_PX_X - XOffset - 1) / TERMINAL_FONT_SCALE));

                if (Enabled) {
                    gFrameBuffer->Base[Y * gFrameBuffer->Width + X].Red      = gTerminal.TextColour.Red;
                    gFrameBuffer->Base[Y * gFrameBuffer->Width + X].Green    = gTerminal.TextColour.Green;
                    gFrameBuffer->Base[Y * gFrameBuffer->Width + X].Blue     = gTerminal.TextColour.Blue;
                    gFrameBuffer->Base[Y * gFrameBuffer->Width + X].Reserved = 0xFF;
                }
            }
        }
    }
    gTerminal.Position = NextPosition;
}

static char gScratch[1024] = {0};

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

void Printf(const char *Format, ...) __attribute__((format(printf, 1, 2)));
void Printf(const char *Format, ...) {
    DisableInterrupts();
    va_list args;
    va_start(args, Format);

    stbsp_vsnprintf(gScratch, sizeof(gScratch), Format, args);
    va_end(args);
    Print(gScratch);
    DisableInterrupts();
}

void PrintLinef(const char *Format, ...) __attribute__((format(printf, 1, 2)));
void PrintLinef(const char *Format, ...) {
    DisableInterrupts();
    va_list args;
    va_start(args, Format);

    stbsp_vsnprintf(gScratch, sizeof(gScratch), Format, args);
    va_end(args);
    PrintLine(gScratch);
    EnableInterrupts();
}

void PrintLinefv(const char *Format, va_list va);
void PrintLinefv(const char *Format, va_list va) {
    DisableInterrupts();
    stbsp_vsnprintf(gScratch, sizeof(gScratch), Format, va);
    PrintLine(gScratch);
    EnableInterrupts();
}

#ifdef KERNEL_DEBUG
void DebugLinef(const char *Format, ...) __attribute__((format(printf, 1, 2)));
void DebugLinef(const char *Format, ...) {
    gTerminal.TextColourInit = true;
    gTerminal.TextColour     = TERMINAL_DEBUG_COLOUR;
    va_list args;
    va_start(args, Format);

    stbsp_vsnprintf(gScratch, sizeof(gScratch), Format, args);
    va_end(args);
    PrintLine(gScratch);
    gTerminal.TextColour = TERMINAL_DEFAULT_COLOUR;
}
#else
void DebugLinef(const char *Format, ...) __attribute__((format(printf, 1, 2)));
void DebugLinef(const char *Format, ...) {}
#endif
