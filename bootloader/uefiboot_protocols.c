#pragma once
// #include <efi/efi.h>
#include "Uefi/UefiBaseType.h"
#include "Uefi/UefiMultiPhase.h"
#include "Uefi/UefiSpec.h"
#include <Protocol/GraphicsOutput.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/Rng.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/SimplePointer.h>
#include <Uefi.h>
EFI_SYSTEM_TABLE                *ST;
EFI_GRAPHICS_OUTPUT_PROTOCOL    *GOP;
EFI_SIMPLE_POINTER_PROTOCOL     *SPP;
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FSP;
EFI_RNG_PROTOCOL                *RNG;
EFI_GUID   SimpleFileSystemProtocolGUID = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
EFI_GUID   GraphicsOutputProtocolGUID   = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
EFI_GUID   SimplePointerProtocolGUID    = EFI_SIMPLE_POINTER_PROTOCOL_GUID;
EFI_GUID   LoadedImageProtocolGUID      = EFI_LOADED_IMAGE_PROTOCOL_GUID;
EFI_HANDLE ImageHandle;

EFI_FILE_PROTOCOL *Root;
#include "uefiboot_libc.c"

void InitProtocols(EFI_SYSTEM_TABLE *SystemTable, EFI_HANDLE Handle) {
    ST          = SystemTable;
    ImageHandle = Handle;
    ST->ConOut->ClearScreen(ST->ConOut);
    ST->ConIn->Reset(ST->ConIn, FALSE);

    EFI_STATUS Status;

    Status = SystemTable->BootServices->LocateProtocol(&GraphicsOutputProtocolGUID,
                                                       NULL,
                                                       (VOID **) &GOP);
    Print(!EFI_ERROR(Status) ? L" OK: " : L"ERR: ");
    PrintLine(L"Graphics Output Protocol");

    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
    Status = SystemTable->BootServices->HandleProtocol(ImageHandle,
                                                       &LoadedImageProtocolGUID,
                                                       (VOID **) &LoadedImage);
    if (EFI_ERROR(Status))
        Panic(L"Failed to get loaded image protocol");

    Status = SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle,
                                                       &SimpleFileSystemProtocolGUID,
                                                       (VOID **) &FSP);
    Print(!EFI_ERROR(Status) ? L" OK: " : L"ERR: ");
    PrintLine(L"Simple File System Protocol");

    Status = FSP->OpenVolume(FSP, &Root);
    if (EFI_ERROR(Status)) {
        Panic(L"Failed to open volume root");
        return;
    }

    UpdateMemoryMap(true);
}
