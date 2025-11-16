// #include "efi/efipoint.h"
// #include <efi/efi.h>
#include "uefiboot_protocols.c"
#include <Protocol/GraphicsOutput.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/SimplePointer.h>
#include <Uefi.h>

#include "uefiboot_libc.c"

void EnterLine(CHAR16 *Line) {
    if (StrEq(Line, L"ls") || StrEq(Line, L"dir")) {
        ListRoot();
    } else {
        int res = LoadElf(Line);
        if (res != 0) {
            Print(L"[");
            PrintNumber(res);
            Print(L"] ");
        }
    }
}

EFI_STATUS EFIMain(EFI_HANDLE Handle, EFI_SYSTEM_TABLE *SystemTable) {
    InitProtocols(SystemTable, Handle);
    RainbowStripe(1);

    ST->ConOut->EnableCursor(ST->ConOut, TRUE);

    PrintLine(L"Root directory listing:");
    EnterLine(L"ls");

    UINTN  AutorunSize;
    CHAR8 *Autorun = GetFileContents(L"AUTORUN", false, &AutorunSize);
    if (Autorun) {
        Print(L"> ");
        Print(ToChar16(Autorun));
        PrintLine(L" (AUTORUN)");
        EnterLine(ToChar16(Autorun));
    }

    while (TRUE) {
        Print(L"> ");
        CHAR16 *Buffer = ReadLine();
        EnterLine(Buffer);
    }

    return (EFI_STATUS) 0;
}
