#pragma once
#include "../elf_libc.c"

#define PCI_ADDRESS_PORT 0x0CF8
#define PCI_DATA_PORT 0x0CFC

u32 ReadPCIPortRaw(u8 Bus, u8 Device, u8 Function, u8 Offset, u8 Length) {
    u32 Value = 0x80000000 | (Bus << 16) | (Device << 11) | (Function << 8) | (Offset & 0xFC);
    OutDWord(PCI_ADDRESS_PORT, Value);

    u32 Data = InDWord(PCI_DATA_PORT);

    u8 Shift = (Offset & 3) * 8;
    Data >>= Shift;

    u32 Mask = (Length == 4) ? 0xFFFFFFFF : ((1U << (Length * 8)) - 1);
    return Data & Mask;
}

void WritePCIPortRaw(u8 Bus, u8 Device, u8 Function, u8 Offset, u32 Value, u8 Length) {
    u32 Address = 0x80000000 | ((u32) Bus << 16) | ((u32) Device << 11) | ((u32) Function << 8) | (Offset & 0xFC);

    OutDWord(PCI_ADDRESS_PORT, Address);

    u32 Shift = (Offset & 3) * 8;
    u32 Mask  = (Length == 4) ? 0xFFFFFFFF : ((1U << (Length * 8)) - 1) << Shift;

    u32 Data;
    if (Length != 4) {
        Data = InDWord(PCI_DATA_PORT);
        Data = (Data & ~Mask) | ((Value << Shift) & Mask);
    } else {
        Data = Value;
    }

    OutDWord(PCI_DATA_PORT, Data);
}

struct PCIConfig {
    u8  Bus;
    u8  Device;
    u8  Interface;
    u8  Function;
    u16 VendorID;
    u16 DeviceID;
    u8  SubClass;
    u8  ClassCode;
    u8  HeaderType;
    u32 BaseAddress;
    u8  IRQLine;
};

u32 ReadPCIPort(struct PCIConfig *Config, u8 Offset, u8 Length) {
    return ReadPCIPortRaw(Config->Bus, Config->Device, Config->Function, Offset, Length);
}

void WritePCIPort(struct PCIConfig *Config, u8 Offset, u32 Value, u8 Length) {
    WritePCIPortRaw(Config->Bus, Config->Device, Config->Function, Offset, Value, Length);
}

void PCIReadBaseAndSize(struct PCIConfig *Config, u32 *Base, u32 *Size) {
    u32 OldBase = ReadPCIPortRaw(Config->Bus, Config->Device, Config->Function, 0x10, 4);
    ;
    WritePCIPortRaw(Config->Bus, Config->Device, Config->Function, 0x10, 0xFFFFFFFF, 4);
    u32 SizeMask = ReadPCIPortRaw(Config->Bus, Config->Device, Config->Function, 0x10, 4);
    ;
    WritePCIPortRaw(Config->Bus, Config->Device, Config->Function, 0x10, OldBase, 4);

    *Base = Config->BaseAddress & ~0xF;
    *Size = ~(SizeMask & ~0xF) + 1;
}

bool ReadPCIConfig(struct PCIConfig *Config, u8 Bus, u8 Device, u8 Function) {
    MemSet(Config, 0, sizeof(struct PCIConfig));
    Config->VendorID = ReadPCIPortRaw(Bus, Device, Function, 0, 2);
    if (Config->VendorID == 0xFFFF) {
        // Device does not exist
        return false;
    }
    Config->Bus         = Bus;
    Config->Device      = Device;
    Config->Function    = Function;
    Config->DeviceID    = ReadPCIPort(Config, 0x2, 2);
    Config->VendorID    = ReadPCIPort(Config, 0x0, 2);
    Config->Interface   = ReadPCIPort(Config, 0x9, 1);
    Config->SubClass    = ReadPCIPort(Config, 0xA, 1);
    Config->ClassCode   = ReadPCIPort(Config, 0xB, 1);
    Config->HeaderType  = ReadPCIPort(Config, 0xE, 1);
    Config->BaseAddress = ReadPCIPort(Config, 0x10, 4);
    Config->IRQLine     = ReadPCIPort(Config, 0x3C, 1);
    return true;
}

#define PCI_MAX_BUSSES 256
#define PCI_MAX_DEVICES 32
#define PCI_MAX_FUNCTIONS 8

#include "./Drivers/USB.c"

void ScanPCIDevice(struct PCIConfig *Config) {
    DebugLinef("+PCI Device (%X:%X): Class = 0x%X, Subclass = 0x%X, Base = 0x%X @%d:%d:%d (IRQ%d)",
               Config->VendorID,
               Config->DeviceID,
               Config->ClassCode,
               Config->SubClass,
               Config->BaseAddress,
               Config->Bus,
               Config->Device,
               Config->Function,
               Config->IRQLine);

    if (Config->ClassCode == 0x0C && Config->SubClass == 0x03) {
        InitializeUSBController(Config);
    }
}

void ScanPCIBus(void) {
    // First, scan bus 1, assuming no secondary busses exist
    for (u8 Device = 0; Device < PCI_MAX_DEVICES; ++Device) {
        // Probe function 0, if it doesn't exist, skip the device, if it exists, check other functions if flagged
        struct PCIConfig Config = {0};
        if (!ReadPCIConfig(&Config, 0, Device, 0)) {
            continue;
        }
        ScanPCIDevice(&Config);
        if ((Config.HeaderType & 0b01000000) == 0) {
            // Single function device
            continue;
        }
        for (u8 Function = 1; Function < PCI_MAX_FUNCTIONS; ++Function) {
            struct PCIConfig Config = {0};
            if (!ReadPCIConfig(&Config, 0, Device, 0)) {
                continue;
            }
            ScanPCIDevice(&Config);
        }
    }
}
