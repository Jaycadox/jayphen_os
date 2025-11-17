#pragma once
// #include <efi/efi.h>
#include "Uefi/UefiBaseType.h"
#include "Uefi/UefiMultiPhase.h"
#include "Uefi/UefiSpec.h"
#include <Guid/FileInfo.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/LoadFile.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/SimplePointer.h>
#include <Uefi.h>
#include <stdbool.h>
#include <stddef.h>
int _fltused = 0;

CHAR16 *int32_to_string(INT32 value);
CHAR16 *int64_to_string(INT64 value);
CHAR16 *uint64_to_string(UINT64 value);
CHAR16 *ptr_to_string(void *Pointer);
void   *memcpy(void *dest, const void *src, size_t n) {
    unsigned char       *d = dest;
    const unsigned char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

void *memset(void *dst, int value, size_t count) {
    unsigned char *ptr = (unsigned char *) dst;
    while (count--) {
        *ptr++ = (unsigned char) value;
    }
    return dst;
}

EFI_GRAPHICS_OUTPUT_BLT_PIXEL hue_to_rgb(float hue);
void                          PrintLine(CHAR16 *msg);
void                          Print(CHAR16 *msg);
void                          Panic(CHAR16 *Msg);
void                          PrintNumber(INT64 num);
struct MemoryLayout          *UpdateMemoryMap(bool OutputStatistics);

#include "uefiboot_protocols.c"

void PrintLine(CHAR16 *msg) {
    ST->ConOut->OutputString(ST->ConOut, msg);
    ST->ConOut->OutputString(ST->ConOut, L"\r\n");
}

void Print(CHAR16 *msg) {
    ST->ConOut->OutputString(ST->ConOut, msg);
}

void Panic(CHAR16 *Msg) {
    ST->ConOut->ClearScreen(ST->ConOut);
    ST->ConIn->Reset(ST->ConIn, FALSE);

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL red = hue_to_rgb(0.0);
    GOP->Blt(GOP,
             &red,
             EfiBltVideoFill,
             0,
             0,
             0,
             0,
             GOP->Mode->Info->HorizontalResolution,
             GOP->Mode->Info->VerticalResolution,
             0);
    ST->ConOut->SetAttribute(ST->ConOut, EFI_BACKGROUND_LIGHTGRAY | EFI_RED);
    PrintLine(L" !!! UEFI PANIC !!! ");
    PrintLine(Msg);
    PrintLine(L"\r\n\r\n\r\nPress any key to continue...");

    UINTN         Index;
    ST->BootServices->WaitForEvent(1, &ST->ConIn->WaitForKey, &Index);
    EFI_INPUT_KEY key;
    ST->ConIn->ReadKeyStroke(ST->ConIn, &key);
    PrintLine(L"Restarting system...");
    ST->BootServices->Stall(15000);
    ST->RuntimeServices->ResetSystem(EfiResetCold, 0, 0, NULL);
}

void PrintNumber(INT64 num) {
    Print(int64_to_string(num));
}
void PrintPointer(void *ptr) {
    Print(ptr_to_string(ptr));
}

// See system/elf_libc.c, as these definitions need to match
struct MemoryRegion {
    UINT64 Start;
    UINT64 NumPages;
};

#define MAX_REGIONS 32
struct MemoryLayout {
    UINTN NumConventionalRegions;
    UINTN NumMemoryMappedIORegions;
    UINTN NumPortMappedIORegions;
    UINTN NumACPIRegions;

    struct MemoryRegion ConventionalRegions[MAX_REGIONS];
    struct MemoryRegion MemoryMappedIORegions[MAX_REGIONS];
    struct MemoryRegion PortMappedIORegions[MAX_REGIONS];
    struct MemoryRegion ACPIRegions[MAX_REGIONS];
};

static EFI_MEMORY_DESCRIPTOR *MemoryMap    = NULL;
static struct MemoryLayout    MemoryLayout = {0};
static UINTN                  MapKey       = 0;
struct MemoryLayout          *UpdateMemoryMap(bool OutputStatistics) {
    EFI_STATUS                 Status;
    static UINTN               Size = 0, DescriptorSize = 0;
    static UINT32              DescriptorVersion = 0;
    static struct MemoryLayout MemoryLayout      = {0};
    memset(&MemoryLayout, 0, sizeof(MemoryLayout));
    Status                                       = ST->BootServices->GetMemoryMap(&Size,
                                            MemoryMap,
                                            &MapKey,
                                            &DescriptorSize,
                                            &DescriptorVersion);
    UINTN UsableMB                               = 0;
    if (Status == EFI_BUFFER_TOO_SMALL) {
        Size += DescriptorSize * 2;
        Status = ST->BootServices->AllocatePool(EfiLoaderData, Size, (void **) &MemoryMap);
        if (EFI_ERROR(Status))
            Panic(L"Failed to allocate memory map");
        Status = ST->BootServices->GetMemoryMap(&Size,
                                                MemoryMap,
                                                &MapKey,
                                                &DescriptorSize,
                                                &DescriptorVersion);
        if (EFI_ERROR(Status))
            Panic(L"Failed to load memory map");

        UINTN Entries = Size / DescriptorSize;

        for (UINTN i = 0; i < Entries; ++i) {
            EFI_MEMORY_DESCRIPTOR *Descriptor =
                (EFI_MEMORY_DESCRIPTOR *) ((UINT8 *) MemoryMap + (i * DescriptorSize));
            switch (Descriptor->Type) {
            case EfiConventionalMemory:
                UsableMB += ((Descriptor->NumberOfPages * (4096)) / 1048576);
                MemoryLayout.ConventionalRegions[MemoryLayout.NumConventionalRegions++] =
                    (struct MemoryRegion) {.Start    = Descriptor->PhysicalStart,
                                                    .NumPages = Descriptor->NumberOfPages};
                break;
            case EfiMemoryMappedIO:
                MemoryLayout.MemoryMappedIORegions[MemoryLayout.NumMemoryMappedIORegions++] =
                    (struct MemoryRegion) {.Start    = Descriptor->PhysicalStart,
                                                    .NumPages = Descriptor->NumberOfPages};
                break;
            case EfiMemoryMappedIOPortSpace:
                MemoryLayout.PortMappedIORegions[MemoryLayout.NumPortMappedIORegions++] =
                    (struct MemoryRegion) {.Start    = Descriptor->PhysicalStart,
                                                    .NumPages = Descriptor->NumberOfPages};
                break;
            case EfiACPIReclaimMemory:
                MemoryLayout.ACPIRegions[MemoryLayout.NumACPIRegions++] =
                    (struct MemoryRegion) {.Start    = Descriptor->PhysicalStart,
                                                    .NumPages = Descriptor->NumberOfPages};
                break;
            };
        }
    }

    if (MemoryLayout.NumMemoryMappedIORegions > MAX_REGIONS ||
        MemoryLayout.NumPortMappedIORegions > MAX_REGIONS ||
        MemoryLayout.NumACPIRegions > MAX_REGIONS ||
        MemoryLayout.NumConventionalRegions > MAX_REGIONS) {
        Panic(L"Too many memory regions");
    }

    if (OutputStatistics) {
        Print(L"Usable memory: ");
        PrintNumber(UsableMB);
        PrintLine(L"MB.");
    }

    return &MemoryLayout;
}

struct FrameBufferInfo {
    UINT32 *Base;
    UINTN   Size;

    UINT32 Width, Height;
    // UINT32 PixelsPerScanline;
};

struct FrameBufferInfo *GetFramebufferInfo(void) {
    static struct FrameBufferInfo Info = {0};
    Info.Base                          = (void *) GOP->Mode->FrameBufferBase;
    Info.Size                          = GOP->Mode->FrameBufferSize;
    Info.Width                         = GOP->Mode->Info->HorizontalResolution;
    Info.Height                        = GOP->Mode->Info->VerticalResolution;
    // Info.PixelsPerScanline             = GOP->Mode->Info->PixelsPerScanLine;

    if (GOP->Mode->Info->PixelFormat != PixelBlueGreenRedReserved8BitPerColor) {
        Panic(L"Unsupported pixel format");
    }

    if (Info.Width != GOP->Mode->Info->PixelsPerScanLine) {
        Panic(L"Framebuffer width and scanline length mismatch");
    }

    return &Info;
}

UINTN StrLen(CHAR16 *str) {
    UINTN len = 0;
    while (str[len] != L'\0') {
        len++;
    }
    return len;
}

bool StrEq(CHAR16 *str1, CHAR16 *str2) {
    while (*str1 && *str2) {
        if (*str1 != *str2) {
            return FALSE;
        }
        str1++;
        str2++;
    }
    return *str1 == *str2;
}

CHAR16 *int32_to_string(INT32 value) {
    static CHAR16 buffers[8][12]; // 8 rotating buffers
    static int    buffer_index = 0;

    CHAR16 *buffer = buffers[buffer_index];
    buffer_index   = (buffer_index + 1) % 8;

    int    i           = 0;
    int    is_negative = 0;
    UINT32 num;

    // Handle zero
    if (value == 0) {
        buffer[0] = L'0';
        buffer[1] = 0;
        return buffer;
    }

    // Handle negative numbers
    if (value < 0) {
        is_negative = 1;
        num         = ~((UINT32) value) + 1;
    } else {
        num = (UINT32) value;
    }

    // Build digits in reverse order
    while (num > 0) {
        buffer[i++] = L'0' + (num % 10);
        num /= 10;
    }

    // Add negative sign if needed
    if (is_negative) {
        buffer[i++] = L'-';
    }

    // Reverse the string
    int start = 0;
    int end   = i - 1;
    while (start < end) {
        CHAR16 temp   = buffer[start];
        buffer[start] = buffer[end];
        buffer[end]   = temp;
        start++;
        end--;
    }

    // Null terminate
    buffer[i] = 0;

    return buffer;
}

CHAR16 *int64_to_string(INT64 value) {
    static CHAR16 buffers[8][22]; // 8 rotating buffers, enough for 19 digits + sign + null
    static int    buffer_index = 0;

    CHAR16 *buffer = buffers[buffer_index];
    buffer_index   = (buffer_index + 1) % 8;

    int    i           = 0;
    int    is_negative = 0;
    UINT64 num;

    // Handle zero
    if (value == 0) {
        buffer[0] = L'0';
        buffer[1] = 0;
        return buffer;
    }

    // Handle negative numbers
    if (value < 0) {
        is_negative = 1;
        num         = ~((UINT64) value) + 1; // two’s complement
    } else {
        num = (UINT64) value;
    }

    // Build digits in reverse order
    while (num > 0) {
        buffer[i++] = L'0' + (num % 10);
        num /= 10;
    }

    // Add negative sign if needed
    if (is_negative)
        buffer[i++] = L'-';

    // Reverse the string
    int start = 0;
    int end   = i - 1;
    while (start < end) {
        CHAR16 temp   = buffer[start];
        buffer[start] = buffer[end];
        buffer[end]   = temp;
        start++;
        end--;
    }

    // Null-terminate
    buffer[i] = 0;

    return buffer;
}

CHAR16 *ptr_to_string(void *Pointer) {
    static CHAR16 buffers[8][20]; // "0x" + 16 hex digits + null
    static int    buffer_index = 0;

    CHAR16 *buffer = buffers[buffer_index];
    buffer_index   = (buffer_index + 1) % 8;

    UINT64 value = (UINT64) Pointer;

    buffer[0] = L'0';
    buffer[1] = L'x';

    for (int i = 0; i < 16; i++) {
        UINT64 nibble = (value >> ((15 - i) * 4)) & 0xF;
        buffer[2 + i] = (nibble < 10) ? (CHAR16) (L'0' + nibble) : (CHAR16) (L'A' + (nibble - 10));
    }

    buffer[18] = 0;

    return buffer;
}

EFI_GRAPHICS_OUTPUT_BLT_PIXEL hue_to_rgb(float hue) {
    while (hue < 0.0f)
        hue += 360.0f;
    while (hue >= 360.0f)
        hue -= 360.0f;

    float h = hue / 60.0f;
    int   i = (int) h; // sector 0–5
    float f = h - i;   // fractional part

    float q = 1.0f - f;
    float t = f;

    float r, g, b;
    switch (i) {
    case 0:
        r = 1.0f;
        g = t;
        b = 0.0f;
        break;
    case 1:
        r = q;
        g = 1.0f;
        b = 0.0f;
        break;
    case 2:
        r = 0.0f;
        g = 1.0f;
        b = t;
        break;
    case 3:
        r = 0.0f;
        g = q;
        b = 1.0f;
        break;
    case 4:
        r = t;
        g = 0.0f;
        b = 1.0f;
        break;
    default: // case 5
        r = 1.0f;
        g = 0.0f;
        b = q;
        break;
    }

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL out;
    out.Red   = (UINT8) (r * 255.0f + 0.5f);
    out.Green = (UINT8) (g * 255.0f + 0.5f);
    out.Blue  = (UINT8) (b * 255.0f + 0.5f);
    return out;
}

void RainbowStripe(int height) {
    const int num_stripes  = 150;
    float     hue_change   = 360.0f / num_stripes;
    UINT32    stripe_width = GOP->Mode->Info->HorizontalResolution / num_stripes;

    float  hue = 0.0f;
    UINT32 x   = 0;
    for (UINT32 i = 0; i < num_stripes; ++i) {
        EFI_GRAPHICS_OUTPUT_BLT_PIXEL pixel = hue_to_rgb(hue);

        GOP->Blt(GOP, &pixel, EfiBltVideoFill, 0, 0, x, 0, stripe_width, height, 0);
        hue += hue_change;
        x += stripe_width;
    }
}

void ListRoot(void) {
    if (Root == NULL) {
        Panic(L"Root is NULL!");
    }

    EFI_FILE_INFO *File;
    UINTN          BufSize = 1024; // Larger buffer
    EFI_STATUS     Status = ST->BootServices->AllocatePool(EfiLoaderData, BufSize, (VOID **) &File);
    if (EFI_ERROR(Status))
        Panic(L"Failed to allocate buffer");

    Status      = Root->SetPosition(Root, 0);
    if (EFI_ERROR(Status))
        Panic(L"Failed to set position");

    for (;;) {
        UINTN ReadSize = BufSize;

        Status = Root->Read(Root, &ReadSize, File);
        if (EFI_ERROR(Status))
            Panic(L"Read failed");
        if (ReadSize == 0)
            break;
        Print(L"    ");
        Print(File->FileName);
        if (File->Attribute & EFI_FILE_DIRECTORY) {
            PrintLine(L"/");
        } else {
            PrintLine(L"");
        }
    }

    ST->BootServices->FreePool(File);
}

EFI_FILE_INFO *GetFile(CHAR16 *FileName) {
    if (Root == NULL) {
        Panic(L"Root is NULL!");
    }

    EFI_FILE_INFO *File;
    UINTN          BufSize = 1024; // Larger buffer
    EFI_STATUS     Status = ST->BootServices->AllocatePool(EfiLoaderData, BufSize, (VOID **) &File);
    if (EFI_ERROR(Status))
        Panic(L"Failed to allocate buffer");

    Status      = Root->SetPosition(Root, 0);
    if (EFI_ERROR(Status))
        Panic(L"Failed to set position");

    for (;;) {
        UINTN ReadSize = BufSize;

        Status = Root->Read(Root, &ReadSize, File);
        if (EFI_ERROR(Status))
            Panic(L"Read failed");
        if (ReadSize == 0) {
            ST->BootServices->FreePool(File);
            return NULL;
        }

        if (StrEq(File->FileName, FileName)) {
            return File;
        }
    }
    return NULL;
}

char *ReadFile(EFI_FILE_INFO *FileInfo) {
    char *Data;

    EFI_STATUS Status = ST->BootServices->AllocatePool(EfiLoaderData,
                                                       FileInfo->FileSize + 1,
                                                       (VOID **) &Data); // +1 for null terminator
    if (EFI_ERROR(Status))
        Panic(L"Failed to load bytes for reading file");

    EFI_FILE_PROTOCOL *FileHandle;
    Status = Root->Open(Root, &FileHandle, FileInfo->FileName, EFI_FILE_READ_ONLY, 0);
    if (EFI_ERROR(Status)) {
        ST->BootServices->FreePool(Data);
        return NULL;
    }

    UINTN ReadSize = FileInfo->FileSize;
    Status         = FileHandle->Read(FileHandle, &ReadSize, Data);
    if (EFI_ERROR(Status) || ReadSize != FileInfo->FileSize) {
        ST->BootServices->FreePool(Data);
        return NULL;
    }

    Data[FileInfo->FileSize] = '\0';
    if (FileInfo->FileSize > 2) {
        if (Data[FileInfo->FileSize - 1] == '\n' || Data[FileInfo->FileSize - 1] == '\r') {
            Data[FileInfo->FileSize - 1] = '\0';
        }
        if (Data[FileInfo->FileSize - 2] == '\n' || Data[FileInfo->FileSize - 2] == '\r') {
            Data[FileInfo->FileSize - 2] = '\0';
        }
    }

    FileHandle->Close(FileHandle);
    return Data;
}

void FreeFile(char *Data) {
    ST->BootServices->FreePool(Data);
}

char *AllocatePageBoundary(UINTN size) {
    char      *Address;
    EFI_STATUS Status = ST->BootServices->AllocatePages(AllocateAnyPages,
                                                        EfiLoaderCode,
                                                        EFI_SIZE_TO_PAGES(size),
                                                        (EFI_PHYSICAL_ADDRESS *) &Address);
    if (EFI_ERROR(Status)) {
        return NULL;
    } else {
        return (char *) Address;
    }

    return NULL;
}

void Debug(char *msg) {
    static CHAR16 wide_msg[256]; // Static buffer for conversion

    // Convert narrow string to wide string
    int i = 0;
    while (msg[i] != '\0' && i < 255) {
        wide_msg[i] = (CHAR16) msg[i];
        i++;
    }
    wide_msg[i] = L'\0';

    PrintLine(wide_msg);
}

CHAR16 *ToChar16(CHAR8 *Message) {
    static CHAR16 WideMessage[256]; // Static buffer for conversion

    // Convert narrow string to wide string
    int i = 0;
    while (Message[i] != '\0' && i < 255) {
        WideMessage[i] = (CHAR16) Message[i];
        i++;
    }
    WideMessage[i] = L'\0';
    return WideMessage;
}

void DebugSameLine(char *Message) {
    static CHAR16 WideMessage[256]; // Static buffer for conversion

    // Convert narrow string to wide string
    int i = 0;
    while (Message[i] != '\0' && i < 255) {
        WideMessage[i] = (CHAR16) Message[i];
        i++;
    }
    WideMessage[i] = L'\0';

    Print(WideMessage);
}

void DebugNumber(INT64 num) {
    CHAR16 *str = int64_to_string(num);
    Print(str);
}

CHAR16 *ReadLine(void) {
    static CHAR16 buffer[256] = {0};
    static UINTN  idx         = 0;
    for (UINTN i = 0; i < idx; i++) {
        buffer[i] = L'\0'; // Clear buffer
    }
    idx = 0;

    while (TRUE) {
        EFI_EVENT events[1];
        events[0] = ST->ConIn->WaitForKey;

        UINTN      index;
        EFI_STATUS status = ST->BootServices->WaitForEvent(1, events, &index);
        if (status == EFI_SUCCESS && index == 0) {
            EFI_INPUT_KEY key;
            if (ST->ConIn->ReadKeyStroke(ST->ConIn, &key) == EFI_SUCCESS) {
                static CHAR16 char_str[2] = {0, 0};
                char_str[0]               = key.UnicodeChar;
                if (key.UnicodeChar == CHAR_BACKSPACE && idx == 0) {
                    continue;
                }
                if (key.UnicodeChar == CHAR_BACKSPACE) {
                    buffer[idx--] = L'\0';
                } else if (key.UnicodeChar == L'\r') {
                    PrintLine(L"");
                    return buffer;
                } else {
                    buffer[idx++] = key.UnicodeChar;
                }
                Print(char_str);
            }
        }
    }
}

UINT64 ReadLineChar8(CHAR8 *ByteBuffer) {
    CHAR16 *WideBuffer = ReadLine();
    UINTN   Length     = StrLen(WideBuffer);
    // Convert narrow string to wide string
    int i = 0;
    while (WideBuffer[i] != '\0' && i < 255) {
        ByteBuffer[i] = (CHAR8) WideBuffer[i];
        i++;
    }
    ByteBuffer[i] = L'\0';
    return Length;
}

UINT64 *SystemDispatcher(INT64 syscall_number, void *userdata) {
    switch (syscall_number) {
    case 0:
        // Print to the screen
        Debug(userdata);
        break;
    case 1:
        // Print to the screen (no newline)
        DebugSameLine(userdata);
        break;
    case 2:
        // Print number to the screen (no newline)
        DebugNumber((INT64) userdata);
        break;
    case 3:
        // Read line and write it into user buffer
        return (UINT64 *) ReadLineChar8((CHAR8 *) userdata);
    case 4:
        // Provide latest memory map
        return (UINT64 *) UpdateMemoryMap(false);
    case 5:
        // Exit boot services
        UpdateMemoryMap(false);
        if (EFI_ERROR(ST->BootServices->ExitBootServices(ImageHandle, MapKey))) {
            Panic(L"Failed to exit UEFI mode");
        }
        return 0;
    case 6:
        // Get framebuffer info
        return (UINT64 *) GetFramebufferInfo();
    }
    return 0;
}

extern void *SystemInvoke(INT64 syscall_number, void *userdata);
extern int   CallSysVEntrypoint(void *func, void *userdata);

#include "../elf_loader/elfloader.c"

CHAR8 *GetFileContents(CHAR16 *FileName, bool Verbose, UINTN *FileSize) {
    EFI_FILE_INFO *Program = GetFile(FileName);
    if (!Program) {
        if (Verbose) {
            Print(L"Failed to open file: ");
            Print(FileName);
            PrintLine(L"");
        }
        return NULL;
    }
    char *FileData = ReadFile(Program);
    if (!FileData) {
        if (Verbose) {
            Print(L"Failed to read file: ");
            Print(FileName);
            PrintLine(L"");
        }
        return NULL;
    }
    if (FileSize) {
        *FileSize = Program->FileSize;
    }

    return FileData;
}

int LoadElf(CHAR16 *FileName) {
    if (!FileName || StrLen(FileName) == 0)
        return 0;

    UINTN  FileSize;
    CHAR8 *FileData = GetFileContents(FileName, true, &FileSize);
    if (!FileData)
        return -1;

    struct elfloader_Result InitialResult =
        elfloader_read((unsigned char *) FileData, FileSize, NULL, 0);
    if (InitialResult.error) {
        Print(L"Invalid ELF file: ");
        Print(FileName);
        PrintLine(L"");
        return -3;
    }

    char                   *MemoryBase = AllocatePageBoundary(InitialResult.memory_size);
    struct elfloader_Result Result     = elfloader_read((unsigned char *) FileData,
                                                    FileSize,
                                                    (void *) MemoryBase,
                                                    InitialResult.memory_size);
    if (Result.error) {
        Panic(L"Failed to load elf");
    }

    // Call the entrypoint
    int (*entrypoint)(void) = (int (*)(void)) Result.entrypoint;

    int res = CallSysVEntrypoint(entrypoint, SystemInvoke);
    // Free memory
    ST->BootServices->FreePages((EFI_PHYSICAL_ADDRESS) MemoryBase,
                                EFI_SIZE_TO_PAGES(InitialResult.memory_size));
    FreeFile(FileData);

    return res;
}
