// OHCI USB host controller driver
#pragma once
#include "../../elf_libc.c"
#include "../Allocator.c"
#include "../Interrupts.c"
#include "../PCI.c"

#define USB_INTERFACE_UCHI 0x0
#define USB_INTERFACE_OCHI 0x10
#define USB_INTERFACE_ECHI 0x20
#define USB_INTERFACE_XCHI 0x30

#define CONTROL_BULK_LIST_SIZE 16

#define BIT0(i) (((i) >> 0) & 1u)
#define BIT1(i) (((i) >> 1) & 1u)
#define BIT2(i) (((i) >> 2) & 1u)
#define BIT3(i) (((i) >> 3) & 1u)

union OHCIRevisionRegister {
    volatile u32 Value;
    struct __attribute__((packed)) {
        volatile u8  Revision : 8;
        volatile u32 Reserved : 24;
    };
};
_Static_assert(sizeof(union OHCIRevisionRegister) == sizeof(u32), "Revision register wrong size");

union OHCIControlRegister {
    volatile u32 Value;
    struct __attribute__((packed)) {
        volatile u8  ControlBulkServiceRatio : 2;
        volatile u8  PeriodicListEnable : 1;
        volatile u8  IsochronousListEnable : 1;
        volatile u8  ControlListEnable : 1;
        volatile u8  BulkListEnable : 1;
        volatile u8  HostControllerFunctionalState : 2;
        volatile u8  InterruptRouting : 1;
        volatile u8  RemoteWakeupConnected : 1;
        volatile u8  RemoteWakeupEnabled : 1;
        volatile u32 Reserved : 21;
    };
};
_Static_assert(sizeof(union OHCIControlRegister) == sizeof(u32), "Control register wrong size");

union OHCICommandStatusRegister {
    volatile u32 Value;
    struct __attribute__((packed)) {
        volatile u8  HostControllerReset : 1;
        volatile u8  ControlListFilled : 1;
        volatile u8  BulkListFilled : 1;
        volatile u8  OwnershipChangeRequest : 1;
        volatile u16 Reserved1 : 12;
        volatile u8  SchedulingOverrunCount : 2;
        volatile u16 Reserved2 : 14;
    };
};
_Static_assert(sizeof(union OHCICommandStatusRegister) == sizeof(u32), "Command register wrong size");

union OHCIInterruptRegister {
    volatile u32 Value;
    struct __attribute__((packed)) {
        volatile u8  SchedulingOverrun : 1;
        volatile u8  WritebackDoneHead : 1;
        volatile u8  StartOfFrame : 1;
        volatile u8  ResumeDetected : 1;
        volatile u8  UnrecoverableError : 1;
        volatile u8  FrameNumberOverflow : 1;
        volatile u8  RootHubStatusChange : 1;
        volatile u32 Reserved : 23;
        volatile u8  OwnershipChange : 1;
        volatile u8  MasterInterruptEnable : 1;
    };
};
_Static_assert(sizeof(union OHCIInterruptRegister) == sizeof(u32), "Interrupt register wrong size");

union OHCIHCCARegister {
    volatile u32 Value;
    struct __attribute__((packed)) {
        volatile u8  Reserved : 8;
        volatile u32 HCCA_BaseAddress : 24;
    };
};
_Static_assert(sizeof(union OHCIHCCARegister) == sizeof(u32), "HCCA register wrong size");

struct OHCIHCCA {
    u32 EndpointDescriptors[32];
    u16 FrameNumber;
    u16 Pad1;
    u32 DoneHead;
    u8  Reserved[120]; // spec says 116, but we should be 256 bytes
} __attribute__((packed));
_Static_assert(sizeof(struct OHCIHCCA) == 256, "HCCA wrong size");

union OHCIEDPointerRegister {
    volatile u32 Value;
    struct __attribute__((packed)) {
        volatile u8  Reserved : 4;
        volatile u32 Pointer : 28;
    };
};
_Static_assert(sizeof(union OHCIEDPointerRegister) == sizeof(u32), "ED Pointer register wrong size");

union OHCIFMIntervalRegister {
    volatile u32 Value;
    struct __attribute__((packed)) {
        volatile u16 FrameInterval : 14;
        volatile u8  Reserved : 2;
        volatile u16 FSLargestDataPacket : 15;
        volatile u8  FrameIntervalToggle : 1;
    };
};
_Static_assert(sizeof(union OHCIFMIntervalRegister) == sizeof(u32), "Interval register wrong size");

union OHCIFMRemainingRegister {
    volatile u32 Value;
    struct __attribute__((packed)) {
        volatile u16 FrameRemaining : 14;
        volatile u32 Reserved : 17;
        volatile u8  FrameRemainingToggle : 1;
    };
};
_Static_assert(sizeof(union OHCIFMRemainingRegister) == sizeof(u32), "FM Remaining register wrong size");

union OHCIFMNumberRegister {
    volatile u32 Value;
    struct __attribute__((packed)) {
        volatile u16 FrameNumber : 16;
        volatile u16 Reserved : 16;
    };
};
_Static_assert(sizeof(union OHCIFMNumberRegister) == sizeof(u32), "FM Number register wrong size");

union OHCIPeriodicStartRegister {
    volatile u32 Value;
    struct __attribute__((packed)) {
        volatile u16 PeriodicStart : 14;
        volatile u32 Reserved : 18;
    };
};
_Static_assert(sizeof(union OHCIPeriodicStartRegister) == sizeof(u32), "Periodic Start register wrong size");

union OHCILSThresholdRegister {
    volatile u32 Value;
    struct __attribute__((packed)) {
        volatile u16 LSThreshold : 12;
        volatile u32 Reserved : 20;
    };
};
_Static_assert(sizeof(union OHCILSThresholdRegister) == sizeof(u32), "LS Threshold register wrong size");

union OHCIRHDescriptorARegister {
    volatile u32 Value;
    struct __attribute__((packed)) {
        volatile u8  NumberDownstreamPorts : 8;
        volatile u8  PowerSwitchingMode : 1;
        volatile u8  NoPowerSwitching : 1;
        volatile u8  DeviceType : 1;
        volatile u8  OverCurrentProtectionMode : 1;
        volatile u8  NoOverCurrentProtection : 1;
        volatile u16 Reserved : 11;
        volatile u8  PowerOnToPowerGoodTime : 8;
    };
};
_Static_assert(sizeof(union OHCIRHDescriptorARegister) == sizeof(u32), "RH Descriptor A register wrong size");

union OHCIRHDescriptorBRegister {
    volatile u32 Value;
    struct __attribute__((packed)) {
        volatile u16 DeviceRemovable;
        volatile u16 PortPowerControlMask;
    };
};
_Static_assert(sizeof(union OHCIRHDescriptorBRegister) == sizeof(u32), "RH Descriptor B register wrong size");

union OHCIRHStatusRegister {
    volatile u32 Value;
    struct __attribute__((packed)) {
        volatile u8  LocalPowerStatus : 1;
        volatile u8  OverCurrentIndicator : 1;
        volatile u16 Reserved1 : 13;
        volatile u8  DeviceRemoteWakeupEnable : 1;
        volatile u8  LocalPowerStatusChange : 1;
        volatile u8  OverCurrentIndicatorChange : 1;
        volatile u16 Reserved2 : 13;
        volatile u8  ClearRemoteWakeupEnable : 1;
    };
};
_Static_assert(sizeof(union OHCIRHStatusRegister) == sizeof(u32), "RH Status register wrong size");

union OHCIPortStatusRegister {
    volatile u32 Value;
    struct __attribute__((packed)) {
        volatile u8  CurrentConnectStatus : 1;
        volatile u8  PortEnableStatus : 1;
        volatile u8  PortSuspendStatus : 1;
        volatile u8  PortOverCurrentIndicator : 1;
        volatile u8  PortResetStatus : 1;
        volatile u8  Reserved1 : 3;
        volatile u8  PortPowerStatus : 1;
        volatile u8  LowSpeedDeviceAttached : 1;
        volatile u8  Reserved2 : 6;
        volatile u8  ConnectStatusChange : 1;
        volatile u8  PortEnableStatusChange : 1;
        volatile u8  PortSuspendStatusChange : 1;
        volatile u8  PortOverCurrentIndicatorChange : 1;
        volatile u8  PortResetStatusChange : 1;
        volatile u16 Reserved3 : 11;
    };
};
_Static_assert(sizeof(union OHCIPortStatusRegister) == sizeof(u32), "Port Status register wrong size");

struct OHCIControllerRegisters {
    volatile union OHCIRevisionRegister      Revision;
    volatile union OHCIControlRegister       Control;
    volatile union OHCICommandStatusRegister CommandStatus;
    volatile union OHCIInterruptRegister     InterruptStatus;
    volatile union OHCIInterruptRegister     InterruptEnable;
    volatile union OHCIInterruptRegister     InterruptDisable;
    volatile union OHCIHCCARegister          HCCA;
    volatile union OHCIEDPointerRegister     PeriodCurrentED;
    volatile union OHCIEDPointerRegister     ControlHeadED;
    volatile union OHCIEDPointerRegister     ControlCurrentED;
    volatile union OHCIEDPointerRegister     BulkHeadED;
    volatile union OHCIEDPointerRegister     BulkCurrentED;
    volatile union OHCIEDPointerRegister     DoneHead;
    volatile union OHCIFMIntervalRegister    FMInterval;
    volatile union OHCIFMRemainingRegister   FMRemaining;
    volatile union OHCIFMNumberRegister      FMNumber;
    volatile union OHCIPeriodicStartRegister PeriodicStart;
    volatile union OHCILSThresholdRegister   LSThreshold;
    volatile union OHCIRHDescriptorARegister RHDescriptorA;
    volatile union OHCIRHDescriptorBRegister RHDescriptorB;
    volatile union OHCIRHStatusRegister      RHStatus;
    volatile union OHCIPortStatusRegister    PortStatus[];
};

union OHCIEDControl {
    volatile u32 Value;
    struct __attribute__((packed)) {
        volatile u32 FunctionAddress : 7;
        volatile u32 EndpointNumber : 4;
        volatile u32 Direction : 2;
        volatile u32 LowSpeed : 1;
        volatile u32 Skip : 1;
        volatile u32 Format : 1;
        volatile u32 MaxPacketSize : 11;
        volatile u32 Available : 5;
    };
};

union OHCIEDHeadPointer {
    volatile u32 Value;
    struct __attribute__((packed)) {
        volatile u32 Halted : 1;
        volatile u32 ToggleCarry : 1;
        volatile u32 Reserved : 2;
        volatile u32 HeadPointer : 28;
    };
};

struct OHCIEndpointDescriptor {
    volatile union OHCIEDControl     Control;
    volatile u32                     TailPointer;
    volatile union OHCIEDHeadPointer HeadPointer;
    volatile u32                     NextED;
};
_Static_assert(sizeof(struct OHCIEndpointDescriptor) == 16, "OHCI ED Size Mismatch");

union OHCIGTDControl {
    volatile u32 Value;
    struct __attribute__((packed)) {
        volatile u32 Reserved : 18;
        volatile u32 BufferRounding : 1;
        volatile u32 DirectionPID : 2;
        volatile u32 DelayInterrupt : 3;
        volatile u32 DataToggle : 2;
        volatile u32 ErrorCount : 2;
        volatile u32 ConditionCode : 4;
    };
};

struct OHCIGeneralTransferDescriptor {
    volatile union OHCIGTDControl Control;
    volatile u32                  CurrentBufferPointer;
    volatile u32                  NextTD;
    volatile u32                  BufferEnd;
};

_Static_assert(sizeof(struct OHCIGeneralTransferDescriptor) == 16, "OHCI GTD Size Mismatch");

struct OHCIDeviceDescriptor {
    u8  Length;
    u8  Type;
    u16 ReleaseNumber;
    u8  DeviceClass;
    u8  SubClass;
    u8  Protocol;
    u8  MaxPacketSize;
    u16 VendorID;
    u16 ProductID;
    u16 DeviceRel;
    u8  ManufacturerIndex;
    u8  ProductIndex;
    u8  SerialNumberIndex;
    u8  Configurations;
} __attribute__((packed));

struct OHCIRequestPacket {
    u8  RequestType;
    u8  Request;
    u16 Value;
    u16 Index;
    u16 Count;
} __attribute__((packed));

struct OHCIRequestStorage {
    struct OHCIEndpointDescriptor        Endpoint;
    struct OHCIGeneralTransferDescriptor TransferDescriptor[64];
    struct OHCIRequestPacket             Request;
};

static struct OHCIControllerRegisters *gOHCIRegisters = NULL;
static struct {
    struct OHCIHCCA               *HCCABase;
    struct OHCIEndpointDescriptor *PeriodicEDStorage;

    struct OHCIEndpointDescriptor *ControlHead;
    struct OHCIEndpointDescriptor *BulkHead;

    struct OHCIRequestStorage *RequestStorage;
} gOHCI = {0};

struct OHCIConfigDescriptor {
    u8  Length;
    u8  Type;
    u16 TotalLength; // Total size including all interfaces/endpoints
    u8  NumInterfaces;
    u8  ConfigurationValue;
    u8  iConfiguration; // String index
    u8  Attributes;     // Bit 7=reserved(1), Bit 6=self-powered, Bit 5=remote wakeup
    u8  MaxPower;       // In 2mA units
} __attribute__((packed));

struct OHCIInterfaceDescriptor {
    u8 Length;
    u8 Type;
    u8 InterfaceNumber;
    u8 AlternateSetting;
    u8 NumEndpoints;
    u8 InterfaceClass;    // 0x03 for HID
    u8 InterfaceSubClass; // 0x01 for boot devices
    u8 InterfaceProtocol; // 0x01 keyboard, 0x02 mouse
    u8 iInterface;        // String index
} __attribute__((packed));

struct OHCIUSBEndpointDescriptor {
    u8  Length;
    u8  Type;
    u8  EndpointAddress; // Bit 7=direction, Bits 3-0=endpoint number
    u8  Attributes;      // Bits 1-0=transfer type (0=control, 1=iso, 2=bulk, 3=interrupt)
    u16 MaxPacketSize;
    u8  Interval; // Polling interval in ms
} __attribute__((packed));

__attribute__((interrupt)) void OHCIInterruptHandler(struct CPUInterruptFrame *Frame);
void OHCISendControlRequest(u8 FunctionAddress, u8 RequestType, u8 Request, u16 Value, u16 Index, u16 Count, void *DataBuffer, u16 MaxPacketSize);
void InitializeUSBController(struct PCIConfig *Config) {
    Print("Detected ");
    switch (Config->Interface) {
    case USB_INTERFACE_UCHI:
        Print("UCHI");
        break;
    case USB_INTERFACE_OCHI:
        Print("OCHI");
        break;
    case USB_INTERFACE_ECHI:
        Print("ECHI");
        break;
    case USB_INTERFACE_XCHI:
        Print("xCHI");
        break;
    }
    Print(" USB controller ");

    if (Config->Interface != USB_INTERFACE_OCHI) {
        PrintLine("(UNSUPPORTED)");
        return;
    } else {
        PrintLine("(OK)");
    }

    if (Config->BaseAddress & 1) {
        Panic("Invalid OHCI controller configuration, bad base address alignment");
    }

    u32 Base, Size;
    PCIReadBaseAndSize(Config, &Base, &Size);

    u16 Command = ReadPCIPort(Config, 0x4, 2); // Command register
    Command |= (1 << 1) | (1 << 2);
    WritePCIPort(Config, 0x4, Command, 2);
    SetDriverInterruptDescriptorTableEntry(Config->IRQLine, OHCIInterruptHandler);
    DebugLinef("Installed OHCI controller: Base address = 0x%016X, Size = %d, IRQ = %d", Base, Size, Config->IRQLine);
    gOHCIRegisters = (void *) (usize) Base;

    // Make sure it's really a OHCI host controller
    if ((gOHCIRegisters->Revision.Value & 0xFF) != 0x10) {
        Panic("Invalid OHCI revision: %X", gOHCIRegisters->Revision.Value);
    }

    gOHCIRegisters->CommandStatus.HostControllerReset = 1;
    SleepMS(1);
    if (gOHCIRegisters->CommandStatus.HostControllerReset == 1) {
        Panic("OHCI controller reset was too slow");
    }

    if (gOHCIRegisters->Control.HostControllerFunctionalState != 0b11) {
        Panic("OHCI controller invalid reset (bad functional state)");
    }

    if (gOHCIRegisters->FMInterval.FrameInterval != 0x2EDF) {
        Panic("OHCI controller invalid reset (bad frame interval)");
    }

    gOHCIRegisters->HCCA.Value = 0xFFFFFFFF;
    u32 Alignment              = ~gOHCIRegisters->HCCA.Value + 1;
    if (Alignment > PAGE_SIZE) {
        Panic("OHCI requested memory alignment too large! (%X, Max = %X)", Alignment, PAGE_SIZE);
    }

    u64   NumberOfPages = ((Alignment + sizeof(struct OHCIHCCA)) + (PAGE_SIZE - 1)) / PAGE_SIZE;
    void *HCCABase      = AllocatePages(NumberOfPages);
    MemSet(HCCABase, 0, NumberOfPages * PAGE_SIZE);

    gOHCIRegisters->Control.Value = 0x00000000;
    SleepMS(10);
    gOHCIRegisters->Control.Value       = 0x000000C0;
    gOHCIRegisters->FMInterval.Value    = 0xA7782EDF;
    gOHCIRegisters->PeriodicStart.Value = 0x00002A2F;

    u32 NumPorts = gOHCIRegisters->RHDescriptorA.NumberDownstreamPorts;
    DebugLinef("Found %d USB ports", NumPorts);

    gOHCIRegisters->RHDescriptorA.PowerSwitchingMode = 0;
    gOHCIRegisters->RHDescriptorA.NoPowerSwitching   = 1;

    for (u32 Port = 0; Port < NumPorts; ++Port) {
        gOHCIRegisters->RHDescriptorB.PortPowerControlMask |= (1 << (Port + 17));
    }
    gOHCIRegisters->HCCA.HCCA_BaseAddress = (u32) (usize) HCCABase >> 8;
    gOHCI.HCCABase                        = HCCABase;
    gOHCI.ControlHead                     = AllocatePages(1);
    MemSet(gOHCI.ControlHead, 0, PAGE_SIZE);
    gOHCIRegisters->ControlHeadED.Pointer = (u32) (usize) gOHCI.ControlHead >> 4;
    gOHCI.BulkHead                        = AllocatePages(1);
    MemSet(gOHCI.BulkHead, 0, PAGE_SIZE);
    gOHCIRegisters->BulkHeadED.Pointer = (u32) (usize) gOHCI.BulkHead >> 4;

    gOHCIRegisters->ControlCurrentED.Value = 0;
    gOHCIRegisters->BulkCurrentED.Value    = 0;

    gOHCIRegisters->Control.Value         = 0x000006B0;
    gOHCIRegisters->RHStatus.Value        = 0x00008000;
    gOHCIRegisters->InterruptStatus.Value = 0x00000004;
    gOHCIRegisters->InterruptEnable.Value = 0xC000001B;

    for (u32 Port = 0; Port < NumPorts; ++Port) {
        if (!gOHCIRegisters->PortStatus[Port].CurrentConnectStatus)
            continue;
        gOHCIRegisters->PortStatus[Port].PortResetStatus = 1;
        do {
            SleepMS(10);
        } while (gOHCIRegisters->PortStatus[Port].PortResetStatusChange != 1);
        gOHCIRegisters->PortStatus[Port].PortResetStatusChange = 1;
        if (gOHCIRegisters->PortStatus[Port].LowSpeedDeviceAttached) {
            DebugLinef("+USB Port[%d/%d]: Mode = Low Speed", Port + 1, NumPorts);
        } else {
            DebugLinef("+USB Port[%d/%d]: Mode = Full Speed", Port + 1, NumPorts);
        }
        gOHCIRegisters->PortStatus[Port].PortEnableStatus = 1;
    }

    // Create control ED list
    struct OHCIEndpointDescriptor *Current = gOHCI.ControlHead;
    for (u32 i = 0; i < CONTROL_BULK_LIST_SIZE; ++i) {
        Current->NextED       = (u32) (usize) (Current + 1);
        Current->Control.Skip = 1;
        ++Current;
    }
    gOHCI.ControlHead[CONTROL_BULK_LIST_SIZE - 1].NextED = 0;

    // Create Bulk ED list
    Current = gOHCI.BulkHead;
    for (u32 i = 0; i < CONTROL_BULK_LIST_SIZE; ++i) {
        Current->NextED       = (u32) (usize) (Current + 1);
        Current->TailPointer  = 0;
        Current->Control.Skip = 1;
        ++Current;
    }
    gOHCI.BulkHead[CONTROL_BULK_LIST_SIZE - 1].NextED = 0;

    // Create tree of periodic EDs
    gOHCI.PeriodicEDStorage = AllocatePages(1);
    for (u32 i = 0; i < 32; ++i) {
        u32 Row1Index                          = BIT0(i) * 8 + BIT1(i) * 4 + BIT2(i) * 2 + BIT3(i) * 1;
        gOHCI.HCCABase->EndpointDescriptors[i] = (u32) (uintptr_t) &gOHCI.PeriodicEDStorage[Row1Index];

        u32 Row2Index                             = BIT0(i) * 4 + BIT1(i) * 2 + BIT2(i) * 1 + 16;
        gOHCI.PeriodicEDStorage[Row1Index].NextED = (u32) (uintptr_t) &gOHCI.PeriodicEDStorage[Row2Index];

        u32 Row3Index                             = BIT0(i) * 2 + BIT1(i) * 1 + 16 + 8;
        gOHCI.PeriodicEDStorage[Row2Index].NextED = (u32) (uintptr_t) &gOHCI.PeriodicEDStorage[Row3Index];

        u32 Row4Index                             = BIT0(i) * 1 + 16 + 8 + 4;
        gOHCI.PeriodicEDStorage[Row3Index].NextED = (u32) (uintptr_t) &gOHCI.PeriodicEDStorage[Row4Index];

        u32 Row5Index                             = 16 + 8 + 4 + 2;
        gOHCI.PeriodicEDStorage[Row4Index].NextED = (u32) (uintptr_t) &gOHCI.PeriodicEDStorage[Row5Index];
    }
    gOHCI.RequestStorage = AllocatePages(1);
    MemSet(gOHCI.RequestStorage, 0, PAGE_SIZE);

    gOHCI.ControlHead[0].NextED           = (u32) (usize) (&gOHCI.RequestStorage->Endpoint);
    gOHCI.RequestStorage->Endpoint.NextED = (u32) (usize) (&gOHCI.ControlHead[1]);
    gOHCI.ControlHead[0].Control.Skip     = 0;

    // --- Initialize Endpoint Descriptor ---
    gOHCI.RequestStorage->Endpoint.Control.FunctionAddress = 0;
    gOHCI.RequestStorage->Endpoint.Control.EndpointNumber  = 0;
    gOHCI.RequestStorage->Endpoint.Control.Direction       = 0b00;
    gOHCI.RequestStorage->Endpoint.Control.LowSpeed        = 0b0;
    gOHCI.RequestStorage->Endpoint.Control.Skip            = 0;
    gOHCI.RequestStorage->Endpoint.Control.Format          = 0;
    gOHCI.RequestStorage->Endpoint.Control.MaxPacketSize   = 64;
    gOHCI.RequestStorage->Endpoint.HeadPointer.ToggleCarry = 0;
    gOHCI.RequestStorage->Endpoint.HeadPointer.Halted      = 0;

    gOHCIRegisters->CommandStatus.ControlListFilled = 1;
    gOHCIRegisters->Control.Value                   = 0x0000190;

    struct OHCIDeviceDescriptor Descriptors[64] = {0};

    for (u32 PortNum = 0; PortNum < NumPorts; ++PortNum) {
        if (!gOHCIRegisters->PortStatus[PortNum].CurrentConnectStatus)
            continue;

        struct OHCIDeviceDescriptor desc;
        gOHCI.RequestStorage->Endpoint.Control.FunctionAddress = 0;
        OHCISendControlRequest(0, 0x80, 0x06, 0x0100, 0, 18, &desc, 0x8);

        u32 DeviceAddress                                      = PortNum + 1;
        gOHCI.RequestStorage->Endpoint.Control.FunctionAddress = 0;
        OHCISendControlRequest(0, 0x00, 0x05, DeviceAddress, 0, 0, NULL, 0x8);

        gOHCI.RequestStorage->Endpoint.Control.FunctionAddress = DeviceAddress;
        OHCISendControlRequest(DeviceAddress, 0x80, 0x06, 0x0100, 0, 18, &desc, 0x8);
        DebugLinef("+USB Device[%d]: VendorID = %04X, ProductID = %04X, Class = %04X, Subclass = %04X, Protocol = %04X, Max packet size = %04X",
                   DeviceAddress,
                   desc.VendorID,
                   desc.ProductID,
                   desc.DeviceClass,
                   desc.SubClass,
                   desc.Protocol,
                   desc.MaxPacketSize);
        Descriptors[DeviceAddress] = desc;
    }

    for (u32 DeviceAddress = 1; DeviceAddress <= NumPorts; ++DeviceAddress) {
        struct OHCIDeviceDescriptor desc;
        OHCISendControlRequest(DeviceAddress, 0x80, 0x06, 0x0100, 0, 18, &desc, 0x8);
        SleepMS(10);

        if (desc.VendorID == 0 && desc.ProductID == 0) {
            continue;
        }

        DebugLinef("+USB Device[%d]: %04X:%04X, MaxPacketSize=%d", DeviceAddress, desc.VendorID, desc.ProductID, desc.MaxPacketSize);

        if (desc.DeviceClass == 0x00) {
            DebugLinef("  Class defined at interface level, reading config...");

            u8 ConfigBuffer[256];
            OHCISendControlRequest(DeviceAddress, 0x80, 0x06, 0x0200, 0, 256, ConfigBuffer, desc.MaxPacketSize);
            SleepMS(10);
        }
    }
}

void OHCISendControlRequest(u8 FunctionAddress, u8 RequestType, u8 Request, u16 Value, u16 Index, u16 Count, void *DataBuffer, u16 MaxPacketSize) {
    gOHCI.RequestStorage->Endpoint.Control.MaxPacketSize   = MaxPacketSize;
    gOHCI.RequestStorage->Endpoint.Control.FunctionAddress = FunctionAddress;
    if (Count > 0 && DataBuffer) {
        MemSet(DataBuffer, 0, Count);
    }

    gOHCIRegisters->Control.ControlListEnable = 0;
    SleepMS(1);
    gOHCIRegisters->ControlCurrentED.Value = 0;

    gOHCI.RequestStorage->Request.RequestType = RequestType;
    gOHCI.RequestStorage->Request.Request     = Request;
    gOHCI.RequestStorage->Request.Value       = Value;
    gOHCI.RequestStorage->Request.Index       = Index;
    gOHCI.RequestStorage->Request.Count       = Count;

    bool IsDeviceToHost = (RequestType & 0x80) != 0;

    u32 NumDataTDs = 0;
    if (Count > 0) {
        NumDataTDs = (Count + MaxPacketSize - 1) / MaxPacketSize;
    }

    u32 StatusTDIndex = 1 + NumDataTDs;
    u32 DummyTDIndex  = StatusTDIndex + 1;

    gOHCI.RequestStorage->TransferDescriptor[0].Control.DirectionPID   = 0b00;
    gOHCI.RequestStorage->TransferDescriptor[0].Control.ConditionCode  = 0;
    gOHCI.RequestStorage->TransferDescriptor[0].Control.ErrorCount     = 0;
    gOHCI.RequestStorage->TransferDescriptor[0].Control.DataToggle     = 0b11;
    gOHCI.RequestStorage->TransferDescriptor[0].Control.DelayInterrupt = 0b111;
    gOHCI.RequestStorage->TransferDescriptor[0].Control.BufferRounding = 0;
    gOHCI.RequestStorage->TransferDescriptor[0].CurrentBufferPointer   = (u32) (usize) (&gOHCI.RequestStorage->Request);
    gOHCI.RequestStorage->TransferDescriptor[0].BufferEnd = (u32) (usize) (&gOHCI.RequestStorage->Request) + sizeof(struct OHCIRequestPacket) - 1;

    if (NumDataTDs > 0) {
        gOHCI.RequestStorage->TransferDescriptor[0].NextTD = (u32) (usize) (&gOHCI.RequestStorage->TransferDescriptor[1]);
    } else {
        gOHCI.RequestStorage->TransferDescriptor[0].NextTD = (u32) (usize) (&gOHCI.RequestStorage->TransferDescriptor[StatusTDIndex]);
    }

    for (u32 i = 0; i < NumDataTDs; ++i) {
        u32 TDIndex        = 1 + i;
        u32 Offset         = i * MaxPacketSize;
        u32 RemainingBytes = Count - Offset;
        u32 ThisPacketSize = (RemainingBytes > MaxPacketSize) ? MaxPacketSize : RemainingBytes;

        gOHCI.RequestStorage->TransferDescriptor[TDIndex].Control.DirectionPID   = IsDeviceToHost ? 0b10 : 0b01;
        gOHCI.RequestStorage->TransferDescriptor[TDIndex].Control.ConditionCode  = 0;
        gOHCI.RequestStorage->TransferDescriptor[TDIndex].Control.ErrorCount     = 0;
        gOHCI.RequestStorage->TransferDescriptor[TDIndex].Control.DelayInterrupt = 0b111;
        gOHCI.RequestStorage->TransferDescriptor[TDIndex].Control.BufferRounding = 1;
        gOHCI.RequestStorage->TransferDescriptor[TDIndex].CurrentBufferPointer   = (u32) ((usize) DataBuffer + Offset);
        gOHCI.RequestStorage->TransferDescriptor[TDIndex].BufferEnd              = (u32) ((usize) DataBuffer + Offset + ThisPacketSize - 1);

        if (i < NumDataTDs - 1) {
            gOHCI.RequestStorage->TransferDescriptor[TDIndex].NextTD = (u32) (usize) (&gOHCI.RequestStorage->TransferDescriptor[TDIndex + 1]);
        } else {
            gOHCI.RequestStorage->TransferDescriptor[TDIndex].NextTD = (u32) (usize) (&gOHCI.RequestStorage->TransferDescriptor[StatusTDIndex]);
        }
    }

    gOHCI.RequestStorage->TransferDescriptor[StatusTDIndex].Control.DirectionPID   = IsDeviceToHost ? 0b01 : 0b10;
    gOHCI.RequestStorage->TransferDescriptor[StatusTDIndex].Control.ConditionCode  = 0;
    gOHCI.RequestStorage->TransferDescriptor[StatusTDIndex].Control.ErrorCount     = 0;
    gOHCI.RequestStorage->TransferDescriptor[StatusTDIndex].Control.DataToggle     = 0b11;
    gOHCI.RequestStorage->TransferDescriptor[StatusTDIndex].Control.DelayInterrupt = 0b000;
    gOHCI.RequestStorage->TransferDescriptor[StatusTDIndex].Control.BufferRounding = 0;
    gOHCI.RequestStorage->TransferDescriptor[StatusTDIndex].CurrentBufferPointer   = 0;
    gOHCI.RequestStorage->TransferDescriptor[StatusTDIndex].BufferEnd              = 0;
    gOHCI.RequestStorage->TransferDescriptor[StatusTDIndex].NextTD = (u32) (usize) (&gOHCI.RequestStorage->TransferDescriptor[DummyTDIndex]);

    MemSet(&gOHCI.RequestStorage->TransferDescriptor[DummyTDIndex], 0, sizeof(struct OHCIGeneralTransferDescriptor));

    gOHCI.RequestStorage->Endpoint.HeadPointer.HeadPointer = (u32) (usize) (&gOHCI.RequestStorage->TransferDescriptor[0]) >> 4;
    gOHCI.RequestStorage->Endpoint.TailPointer             = (u32) (usize) (&gOHCI.RequestStorage->TransferDescriptor[DummyTDIndex]);

    gOHCI.HCCABase->DoneHead                        = 0;
    gOHCIRegisters->InterruptStatus.Value           = 2;
    gOHCIRegisters->CommandStatus.ControlListFilled = 1;
    gOHCIRegisters->Control.ControlListEnable       = 1;

    u32 Timeout = 500;
    while ((gOHCIRegisters->InterruptStatus.Value & 2) == 0 && --Timeout) {
        SleepMS(1);
    }
    if (Timeout == 0) {
        DebugLinef("Timed out");
    }
    gOHCIRegisters->InterruptStatus.Value = 2;
}

void OHCIInterruptHandler(struct CPUInterruptFrame *Frame) {
    u32 Status = gOHCIRegisters->InterruptStatus.Value;
    // DebugLinef("Interrupt %b", Status);
    // gOHCIRegisters->InterruptStatus.Value = Status;
}
