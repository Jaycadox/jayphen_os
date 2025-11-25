
// OHCI USB host controller driver
#pragma once
#include "elf_libc.c"
#include "kernel/Allocator.c"
#include "kernel/Input.c"
#include "kernel/Interrupts.c"
#include "kernel/PCI.c"

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
} __attribute__((packed, aligned(16)));
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
} __attribute__((packed, aligned(16)));

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

struct OHCIConfigDescriptor {
    u8  Length;
    u8  Type;
    u16 TotalLength;
    u8  NumInterfaces;
    u8  ConfigurationValue;
    u8  iConfiguration;
    u8  Attributes;
    u8  MaxPower;
} __attribute__((packed));

struct OHCIInterfaceDescriptor {
    u8 Length;
    u8 Type;
    u8 InterfaceNumber;
    u8 AlternateSetting;
    u8 NumEndpoints;
    u8 InterfaceClass;
    u8 InterfaceSubClass;
    u8 InterfaceProtocol;
    u8 iInterface;
} __attribute__((packed));

struct OHCIHIDDescriptor {
    u8  Length;
    u8  Type;
    u16 HID;
    u8  CountryCode;
    u8  NumDescriptors;
    u8  ReportDescriptorType;
    u16 ReportDescriptorLength;
};

struct OHCIUSBEndpointDescriptor {
    u8  Length;
    u8  Type;
    u8  EndpointAddress;
    u8  Attributes;
    u16 MaxPacketSize;
    u8  Interval;
} __attribute__((packed));

enum USBRequestType {
    USB_DIR_HOST_TO_DEVICE = 0x00,
    USB_DIR_DEVICE_TO_HOST = 0x80,

    USB_TYPE_STANDARD = 0x00,
    USB_TYPE_CLASS    = 0x20,
    USB_TYPE_VENDOR   = 0x40,

    USB_RECIP_DEVICE    = 0x00,
    USB_RECIP_INTERFACE = 0x01,
    USB_RECIP_ENDPOINT  = 0x02,
    USB_RECIP_OTHER     = 0x03,
};

enum USBRequest {
    USB_REQ_GET_STATUS        = 0x00,
    USB_REQ_CLEAR_FEATURE     = 0x01,
    USB_REQ_SET_FEATURE       = 0x03,
    USB_REQ_SET_ADDRESS       = 0x05,
    USB_REQ_GET_DESCRIPTOR    = 0x06,
    USB_REQ_SET_DESCRIPTOR    = 0x07,
    USB_REQ_GET_CONFIGURATION = 0x08,
    USB_REQ_SET_CONFIGURATION = 0x09,
    USB_REQ_GET_INTERFACE     = 0x0A,
    USB_REQ_SET_INTERFACE     = 0x0B,
    USB_REQ_SYNCH_FRAME       = 0x0C,
};

enum USBDescriptorType {
    USB_DESC_DEVICE           = 0x0100,
    USB_DESC_CONFIGURATION    = 0x0200,
    USB_DESC_STRING           = 0x0300,
    USB_DESC_INTERFACE        = 0x0400,
    USB_DESC_ENDPOINT         = 0x0500,
    USB_DESC_DEVICE_QUALIFIER = 0x0600,
    USB_DESC_OTHER_SPEED_CONF = 0x0700,
    USB_DESC_INTERFACE_POWER  = 0x0800,
    USB_DESC_HUB              = 0x2900,
};

enum USBInterfaceClass {
    USB_INTERFACE_UNK          = 0x0,
    USB_INTERFACE_HID          = 0x3,
    USB_INTERFACE_MASS_STORAGE = 0x8,
    USB_INTERFACE_HUB          = 0x9,
};

struct OHCIPort {
    bool FullSpeed;
};

enum USBHIDClass { KEYBOARD, MOUSE };

union HIDKeyboardReport {
    u64 Value;
    struct __attribute__((packed)) {
        volatile u8 LeftCtrl : 1;
        volatile u8 LeftShift : 1;
        volatile u8 LeftAlt : 1;
        volatile u8 LeftGUI : 1;
        volatile u8 RightCtrl : 1;
        volatile u8 RightShift : 1;
        volatile u8 RightAlt : 1;
        volatile u8 RightGUI : 1;
        volatile u8 Reserved;
        volatile u8 KeyCode[6];
    };
};

union HIDMouseReport {
    u32 Value;
    struct __attribute__((packed)) {
        volatile u8 Left : 1;
        volatile u8 Right : 1;
        volatile u8 Middle : 1;
        volatile u8 Button3 : 1;
        volatile u8 Button4 : 1;
        volatile u8 Reserved : 3;
        volatile i8 X;
        volatile i8 Y;
        volatile i8 Wheel;
    };
};

struct OHCIDevice {
    char                            Buffer[256];
    struct OHCIDeviceDescriptor     Descriptor;
    struct OHCIPort                *Port;
    u32                             FunctionAddress;
    bool                            Active;
    bool                            HasConfigAndInterface;
    struct OHCIConfigDescriptor    *Config;
    struct OHCIInterfaceDescriptor *Interface;
    char                            ProductString[64];
    char                            ManufacturerString[64];
    enum USBInterfaceClass          InterfaceClass;
    union {
        struct {
            u8                                    EndpointNumber;
            u8                                    Interval;
            u8                                    Direction;
            struct OHCIEndpointDescriptor        *EndpointDescriptor;
            struct OHCIGeneralTransferDescriptor *TransferDescriptor;
            struct OHCIGeneralTransferDescriptor *DummyTransferDescriptor;
            enum USBHIDClass                      HIDClass;
            union {
                u64                     Data[1];
                union HIDMouseReport    MouseReport;
                union HIDKeyboardReport KeyboardReport;
            };
        } HID;
    };
};

struct OHCIUSBHubDescriptor {
    u8  bLength;
    u8  bDescriptorType;
    u8  bNbrPorts;
    u16 wHubCharacteristics;
    u8  bPwrOn2PwrGood;
    u8  bHubContrCurrent;
} __attribute__((packed));

struct OHCIUSBStringDescriptor {
    u8 Length;
    u8 Type;
    u8 String[128];
};

struct OHCIUSBPortStatus {
    u16 PortStatus;
    u16 PortChange;
} __attribute__((packed));

#define USB_MAX_DEVICES 128

static struct OHCIControllerRegisters *gOHCIRegisters = NULL;
static struct {
    struct OHCIHCCA               *HCCABase;
    struct OHCIEndpointDescriptor *PeriodicEDStorage;

    struct OHCIEndpointDescriptor *ControlHead;
    struct OHCIEndpointDescriptor *BulkHead;

    struct OHCIRequestStorage *RequestStorage;
    struct OHCIPort            Ports[USB_MAX_DEVICES];
    struct OHCIDevice          Devices[USB_MAX_DEVICES];
    usize                      Period16MSIndex;
    usize                      Period8MSIndex;
    usize                      Period4MSIndex;
    usize                      Period2MSIndex;
    usize                      Period1MSIndex;

} gOHCI = {0};

__attribute__((interrupt)) void OHCIInterruptHandler(struct CPUInterruptFrame *Frame);
bool OHCIDeviceSendControlRequest(struct OHCIDevice *Device, u8 RequestType, u8 Request, u16 Value, u16 Index, u16 Count, void *DataBuffer);
bool OHCISendControlRequest(u8 FunctionAddress, u8 RequestType, u8 Request, u16 Value, u16 Index, u16 Count, void *DataBuffer, u16 MaxPacketSize);
bool OHCIInitializeDevice(u32 Port);

void InitializeUSBController(struct PCIConfig *Config) {
    Printf("PCI[%02d:%02d:%02d]: Detected ", Config->Bus, Config->Device, Config->Function);
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
    u32 ResetTimeout                                  = 100;
    while (gOHCIRegisters->CommandStatus.HostControllerReset) {
        SleepMS(1);
        if (--ResetTimeout == 0) {
            Panic("OHCI controller reset timed out");
        }
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

    gOHCIRegisters->Control.Value       = 0x00000000;
    gOHCIRegisters->Control.Value       = 0x000000C0;
    gOHCIRegisters->FMInterval.Value    = 0xA7782EDF;
    gOHCIRegisters->PeriodicStart.Value = 0x00002A2F;

    u32 NumPorts = gOHCIRegisters->RHDescriptorA.NumberDownstreamPorts;
    DebugLinef("Found %d USB ports", NumPorts);

    gOHCIRegisters->RHDescriptorA.PowerSwitchingMode = 0;
    gOHCIRegisters->RHDescriptorA.NoPowerSwitching   = 1;
    SleepMS(gOHCIRegisters->RHDescriptorA.PowerOnToPowerGoodTime * 2);

    for (u32 Port = 0; Port < NumPorts; ++Port) {
        gOHCIRegisters->RHDescriptorB.PortPowerControlMask |= (1 << (Port + 17));
    }
    gOHCIRegisters->HCCA.Value = (u32) (usize) HCCABase;
    gOHCI.HCCABase             = HCCABase;
    gOHCI.ControlHead          = AllocatePages(1);
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
    gOHCIRegisters->InterruptEnable.Value = 0xC0000012;
    for (u32 Port = 0; Port < NumPorts; ++Port) {
        if (!gOHCIRegisters->PortStatus[Port].CurrentConnectStatus)
            continue;
        gOHCIRegisters->PortStatus[Port].PortResetStatus = 1;
        u32 ResetTimeout                                 = 500;
        while (gOHCIRegisters->PortStatus[Port].PortResetStatus) {
            SleepMS(1);
            if (--ResetTimeout == 0) {
                DebugLinef("Port %d reset timed out", Port);
                break;
            }
        }
        gOHCIRegisters->PortStatus[Port].PortResetStatusChange = 1;
        if (gOHCIRegisters->PortStatus[Port].LowSpeedDeviceAttached) {
            DebugLinef("+USB Port[%d/%d]: Mode = Low Speed", Port + 1, NumPorts);
            gOHCI.Ports[Port].FullSpeed = false;
        } else {
            DebugLinef("+USB Port[%d/%d]: Mode = Full Speed", Port + 1, NumPorts);
            gOHCI.Ports[Port].FullSpeed = true;
        }
        gOHCIRegisters->PortStatus[Port].PortEnableStatus = 1;
    }

    // Allow USB devices to "wake up"
    SleepMS(10);

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

    gOHCI.PeriodicEDStorage = AllocatePages(1);
    MemSet(gOHCI.PeriodicEDStorage, 0, PAGE_SIZE);
    for (u32 i = 0; i < 32; ++i) {
        u32 Row1Index                          = BIT0(i) * 8 + BIT1(i) * 4 + BIT2(i) * 2 + BIT3(i) * 1;
        gOHCI.HCCABase->EndpointDescriptors[i] = (u32) (uintptr_t) &gOHCI.PeriodicEDStorage[Row1Index];
        gOHCI.Period16MSIndex                  = Row1Index;

        u32 Row2Index                             = BIT0(i) * 4 + BIT1(i) * 2 + BIT2(i) * 1 + 16;
        gOHCI.PeriodicEDStorage[Row1Index].NextED = (u32) (uintptr_t) &gOHCI.PeriodicEDStorage[Row2Index];
        gOHCI.Period8MSIndex                      = Row2Index;

        u32 Row3Index                             = BIT0(i) * 2 + BIT1(i) * 1 + 16 + 8;
        gOHCI.PeriodicEDStorage[Row2Index].NextED = (u32) (uintptr_t) &gOHCI.PeriodicEDStorage[Row3Index];
        gOHCI.Period4MSIndex                      = Row3Index;

        u32 Row4Index                             = BIT0(i) * 1 + 16 + 8 + 4;
        gOHCI.PeriodicEDStorage[Row3Index].NextED = (u32) (uintptr_t) &gOHCI.PeriodicEDStorage[Row4Index];
        gOHCI.Period2MSIndex                      = Row4Index;

        u32 Row5Index                             = 16 + 8 + 4 + 2;
        gOHCI.PeriodicEDStorage[Row4Index].NextED = (u32) (uintptr_t) &gOHCI.PeriodicEDStorage[Row5Index];
        gOHCI.Period1MSIndex                      = Row5Index;
    }
    for (u32 i = 0; i < 31; i++) {
        gOHCI.PeriodicEDStorage[i].Control.Skip = 1;
    }

    gOHCIRegisters->Control.PeriodicListEnable = 1;

    gOHCI.RequestStorage = AllocatePages(1);
    MemSet(gOHCI.RequestStorage, 0, PAGE_SIZE);

    gOHCI.ControlHead[0].NextED           = (u32) (usize) (&gOHCI.RequestStorage->Endpoint);
    gOHCI.RequestStorage->Endpoint.NextED = (u32) (usize) (&gOHCI.ControlHead[1]);
    gOHCI.ControlHead[0].Control.Skip     = 0;

    gOHCI.RequestStorage->Endpoint.Control.FunctionAddress = 0;
    gOHCI.RequestStorage->Endpoint.Control.EndpointNumber  = 0;
    gOHCI.RequestStorage->Endpoint.Control.Direction       = 0b00;
    gOHCI.RequestStorage->Endpoint.Control.LowSpeed        = 0b0;

    gOHCI.Devices[0].Active = true;
    for (u32 PortNum = 0; PortNum < NumPorts; ++PortNum) {
        if (!gOHCIRegisters->PortStatus[PortNum].CurrentConnectStatus)
            continue;

        if (!OHCIInitializeDevice(PortNum)) {
            DebugLinef("Failed to initialize OHCI port: %d", PortNum);
        }
    }
}

bool OHCIDetectHIDDevice(struct OHCIDevice *Device);
bool OHCIDetectHubDevice(struct OHCIDevice *Device);
bool OHCIDetectMassStorageDevice(struct OHCIDevice *Device);
bool OHCIDeviceGetString(struct OHCIDevice *Device, u8 StringIndex, char *OutString, usize Length);
bool OHCIInitializeDevice(u32 Port) {
    struct OHCIDeviceDescriptor Descriptor;
    if (!OHCISendControlRequest(0,
                                USB_DIR_DEVICE_TO_HOST,
                                USB_REQ_GET_DESCRIPTOR,
                                USB_DESC_DEVICE,
                                0,
                                sizeof(struct OHCIDeviceDescriptor),
                                &Descriptor,
                                0x8)) {
        DebugLinef("OHCI port init failed: 1");
        return false;
    }

    // Find index of first non-Active device in the device list to get new function address
    i32 Address = -1;
    for (u32 i = 0; i < USB_MAX_DEVICES; ++i) {
        if (!gOHCI.Devices[i].Active) {
            Address = i;
            break;
        }
    }
    if (Address == -1) {
        DebugLinef("Could not initialize OHCI USB device: could not allocate a function address (none left?)");
        return false;
    }

    struct OHCIDevice *Device = &gOHCI.Devices[Address];
    Device->Port              = &gOHCI.Ports[Port];
    u32 MaxPacketSize         = Descriptor.MaxPacketSize;
    if (!OHCISendControlRequest(0, USB_DIR_HOST_TO_DEVICE, USB_REQ_SET_ADDRESS, Address, 0, 0, NULL, MaxPacketSize)) {
        DebugLinef("OHCI port init failed: 2");
        return false;
    }

    SleepMS(2);

    // Make sure we can talk to the device on the new address
    if (!OHCISendControlRequest(Address, 0x80, 0x06, 0x0100, 0, 18, &Descriptor, MaxPacketSize)) {
        DebugLinef("OHCI port init failed: 3");
        return false;
    }

    Device->Active          = true;
    Device->Descriptor      = Descriptor;
    Device->FunctionAddress = Address;

    OHCIDeviceGetString(Device, Descriptor.ProductIndex, Device->ProductString, sizeof(Device->ProductString));
    OHCIDeviceGetString(Device, Descriptor.ManufacturerIndex, Device->ManufacturerString, sizeof(Device->ManufacturerString));

    DebugLinef("+USB Device[%d]: VendorID = %04X, ProductID = %04X, Class = %04X, Subclass = %04X, Protocol = %04X, Product = %s, Manufacturer = %s",
               Address,
               Descriptor.VendorID,
               Descriptor.ProductID,
               Descriptor.DeviceClass,
               Descriptor.SubClass,
               Descriptor.Protocol,
               Device->ProductString,
               Device->ManufacturerString);

    if (!OHCIDeviceSendControlRequest(Device,
                                      USB_DIR_DEVICE_TO_HOST,
                                      USB_REQ_GET_DESCRIPTOR,
                                      USB_DESC_CONFIGURATION,
                                      0,
                                      sizeof(Device->Buffer),
                                      &Device->Buffer)) {
        DebugLinef("OHCI port init failed: 4");
        return false;
    }

    struct OHCIConfigDescriptor *Config = (struct OHCIConfigDescriptor *) Device->Buffer;
    DebugLinef("\tInterface Count = %d, Attributes = %b", Config->NumInterfaces, Config->Attributes);
    struct OHCIInterfaceDescriptor *Interface = (struct OHCIInterfaceDescriptor *) (Device->Buffer + Config->Length);
    // TODO: support multiple interfaces
    DebugLinef("\tInterface[0]");
    DebugLinef("\t\tClass = %x, Subclass = %x, Protocol = %x", Interface->InterfaceClass, Interface->InterfaceSubClass, Interface->InterfaceProtocol);

    Device->HasConfigAndInterface = true;
    Device->Config                = Config;
    Device->Interface             = Interface;

    if (gOHCI.Devices[Address].HasConfigAndInterface) {
        switch (gOHCI.Devices[Address].Interface->InterfaceClass) {
        case USB_INTERFACE_HID:
            if (!OHCIDetectHIDDevice(&gOHCI.Devices[Address])) {
                DebugLinef("Failed to initialize HID OHCI device");
            }
            break;
        case USB_INTERFACE_MASS_STORAGE:
            if (!OHCIDetectMassStorageDevice(&gOHCI.Devices[Address])) {
                DebugLinef("Failed to initialize Mass Storage OHCI device");
            }
            break;
        case USB_INTERFACE_HUB:
            if (!OHCIDetectHubDevice(&gOHCI.Devices[Address])) {
                DebugLinef("Failed to initialize hub OHCI device");
            }
            break;
        default:
            DebugLinef("Unsupported USB interface class: %d", gOHCI.Devices[Address].Interface->InterfaceClass);
            return false;
        };
    }
    return true;
}

void ConvertUTF16LEToASCII(u16 *String, i32 Length, char *AsciiOut) {
    for (i32 i = 0; i < Length; i++) {
        AsciiOut[i] = (char) (String[i] & 0xFF);
    }
    AsciiOut[Length] = 0;
}

bool OHCIDeviceGetString(struct OHCIDevice *Device, u8 StringIndex, char *OutString, usize Length) {
    static struct OHCIUSBStringDescriptor Data = {0};
    MemSet(&Data, 0, sizeof(struct OHCIUSBStringDescriptor));
    if (!OHCIDeviceSendControlRequest(Device,
                                      USB_DIR_DEVICE_TO_HOST,
                                      USB_REQ_GET_DESCRIPTOR,
                                      (USB_DESC_STRING) | StringIndex,
                                      0x0409,
                                      sizeof(Data),
                                      &Data)) {
        return false;
    }

    MemSet(OutString, 0, Length);
    i32 CharCount = (Data.Length - 2) / 2;
    ConvertUTF16LEToASCII((u16 *) Data.String, CharCount, OutString);
    return true;
}

bool OHCIDetectHIDDevice(struct OHCIDevice *Device) {
    if (!Device->HasConfigAndInterface)
        return false;
    if (Device->Interface->InterfaceClass != 3)
        return false;
    if (Device->Interface->InterfaceSubClass != 1) {
        DebugLinef("USB HID device was not a boot device, unsupported");
        return false;
    }

    char *DeviceType = NULL;
    switch (Device->Interface->InterfaceProtocol) {
    case 1:
        DeviceType           = "Keyboard";
        Device->HID.HIDClass = KEYBOARD;
        break;
    case 2:
        DeviceType           = "Mouse";
        Device->HID.HIDClass = MOUSE;
        break;
    default:
        DebugLinef("HID device has unsupported protocol number: %d", Device->Interface->InterfaceProtocol);
        return false;
    };
    Device->InterfaceClass = USB_INTERFACE_HID;

    struct OHCIHIDDescriptor         *HIDDescriptor = (struct OHCIHIDDescriptor *) ((u8 *) Device->Interface + Device->Interface->Length);
    struct OHCIUSBEndpointDescriptor *USBED         = (struct OHCIUSBEndpointDescriptor *) ((u8 *) HIDDescriptor + HIDDescriptor->Length);
    Device->HID.EndpointNumber                      = USBED->EndpointAddress & 0x0F;
    Device->HID.Direction                           = (USBED->EndpointAddress & 0x80) >> 7;
    Device->HID.Interval                            = USBED->Interval;

    PrintLinef(
        "USB Device[%d]: %s(%s): Detected as HID device. Type = %s, Country Code = %X, HID = %X, Polling rate = %dms, Endpoint = %d, Direction = %d",
        Device->FunctionAddress,
        Device->ProductString,
        Device->ManufacturerString,
        DeviceType,
        HIDDescriptor->CountryCode,
        HIDDescriptor->HID,
        Device->HID.Interval,
        Device->HID.EndpointNumber,
        Device->HID.Direction);
    OHCIDeviceSendControlRequest(Device, USB_DIR_HOST_TO_DEVICE | USB_TYPE_STANDARD | USB_RECIP_DEVICE, USB_REQ_SET_CONFIGURATION, 1, 0, 0, NULL);

    Device->HID.EndpointDescriptor = AllocatePages(1);
    MemSet(Device->HID.EndpointDescriptor, 0, sizeof(*Device->HID.EndpointDescriptor));
    Device->HID.TransferDescriptor = AllocatePages(1);
    MemSet(Device->HID.TransferDescriptor, 0, sizeof(*Device->HID.TransferDescriptor));
    Device->HID.DummyTransferDescriptor = AllocatePages(1);
    MemSet(Device->HID.DummyTransferDescriptor, 0, sizeof(*Device->HID.DummyTransferDescriptor));

    Device->HID.EndpointDescriptor->Control.FunctionAddress = Device->FunctionAddress;
    Device->HID.EndpointDescriptor->Control.EndpointNumber  = Device->HID.EndpointNumber;
    Device->HID.EndpointDescriptor->Control.Direction       = 0b10; // IN
    Device->HID.EndpointDescriptor->Control.MaxPacketSize   = USBED->MaxPacketSize;
    Device->HID.EndpointDescriptor->HeadPointer.Value       = (u32) ((usize) Device->HID.TransferDescriptor);
    Device->HID.EndpointDescriptor->HeadPointer.ToggleCarry = 1;
    Device->HID.EndpointDescriptor->TailPointer             = (u32) ((usize) Device->HID.DummyTransferDescriptor);

    Device->HID.TransferDescriptor->Control.DirectionPID   = 0b10; // IN
    Device->HID.TransferDescriptor->Control.DelayInterrupt = 0b0;
    Device->HID.TransferDescriptor->Control.DataToggle     = 0b11;
    Device->HID.TransferDescriptor->CurrentBufferPointer   = (u32) (usize) (Device->HID.Data);
    Device->HID.TransferDescriptor->BufferEnd              = (u32) (usize) ((u8 *) Device->HID.Data + USBED->MaxPacketSize - 1);
    Device->HID.TransferDescriptor->NextTD                 = (u32) (usize) Device->HID.DummyTransferDescriptor;

    usize Index = 0;
    if (Device->HID.Interval >= 16) {
        Index = gOHCI.Period16MSIndex;
    } else if (Device->HID.Interval >= 8) {
        Index = gOHCI.Period8MSIndex;
    } else if (Device->HID.Interval >= 4) {
        Index = gOHCI.Period4MSIndex;
    } else if (Device->HID.Interval >= 2) {
        Index = gOHCI.Period2MSIndex;
    } else if (Device->HID.Interval >= 1) {
        Index = gOHCI.Period1MSIndex;
    } else {
        // Assume standard
        Index = gOHCI.Period8MSIndex;
    }

    struct OHCIEndpointDescriptor *ED = &gOHCI.PeriodicEDStorage[Index];

    Device->HID.EndpointDescriptor->NextED = ED->NextED;
    ED->NextED                             = (u32) (usize) Device->HID.EndpointDescriptor;
    return true;
}

bool OHCIDetectMassStorageDevice(struct OHCIDevice *Device) {
    if (!Device->HasConfigAndInterface)
        return false;
    if (Device->Interface->InterfaceClass != 8)
        return false;
    char *CommandSet = NULL;
    switch (Device->Interface->InterfaceSubClass) {
    case 6:
        CommandSet = "SCSI Transparent";
        break;
    default:
        CommandSet = "Unknown";
        break;
    };
    char *Transport = NULL;
    switch (Device->Interface->InterfaceProtocol) {
    case 0x50:
        Transport = "Bulk-only";
        break;
    default:
        Transport = "Unknown";
        break;
    };

    Device->InterfaceClass = USB_INTERFACE_MASS_STORAGE;
    PrintLinef("USB Device[%d]: %s(%s): Detected as Mass-Storage device. Command Set = %s, Transport = %s",
               Device->FunctionAddress,
               Device->ProductString,
               Device->ManufacturerString,
               CommandSet,
               Transport);

    return true;
}

bool OHCIDetectHubDevice(struct OHCIDevice *Device) {
    struct OHCIUSBHubDescriptor HubDesc = {0};
    OHCIDeviceSendControlRequest(Device, 0xA0, USB_REQ_GET_DESCRIPTOR, USB_DESC_HUB, 0, sizeof(struct OHCIUSBHubDescriptor), &HubDesc);

    u8 NumPorts = HubDesc.bNbrPorts;
    PrintLinef("USB Device[%d]: %s(%s): Detected as Hub device. Ports = %d",
               Device->FunctionAddress,
               Device->ProductString,
               Device->ManufacturerString,
               NumPorts);

    for (u32 Port = 1; Port <= NumPorts; Port++) {
        OHCIDeviceSendControlRequest(Device, USB_DIR_HOST_TO_DEVICE | USB_TYPE_CLASS | USB_RECIP_OTHER, 0x03, 8, Port, 0, NULL);
        SleepMS(1);
        OHCIDeviceSendControlRequest(Device, USB_DIR_HOST_TO_DEVICE | USB_TYPE_CLASS | USB_RECIP_OTHER, 0x03, 4, Port, 0, NULL);
    }
    SleepMS(1);

    for (u32 Port = 1; Port <= NumPorts; Port++) {
        struct OHCIUSBPortStatus PortStatus = {0};
        OHCIDeviceSendControlRequest(Device,
                                     USB_DIR_DEVICE_TO_HOST | USB_TYPE_CLASS | USB_RECIP_OTHER,
                                     USB_REQ_GET_STATUS,
                                     0,
                                     Port,
                                     sizeof(struct OHCIUSBPortStatus),
                                     &PortStatus);
        // Port connected bit
        if (!(PortStatus.PortStatus & 0x0001)) {
            continue;
        }

        // Port enabled bit
        if (!(PortStatus.PortStatus & 0x0002)) {
            continue;
        }
        if (!OHCIInitializeDevice(Port)) {
            DebugLinef("Failed to init port");
        }
    }

    Device->InterfaceClass = USB_INTERFACE_HUB;
    return true;
}

bool OHCIDeviceSendControlRequest(struct OHCIDevice *Device, u8 RequestType, u8 Request, u16 Value, u16 Index, u16 Count, void *DataBuffer) {
    u32 MaxPacketSize   = Device->Descriptor.MaxPacketSize;
    u32 FunctionAddress = Device->FunctionAddress;
    return OHCISendControlRequest(FunctionAddress, RequestType, Request, Value, Index, Count, DataBuffer, MaxPacketSize);
}
volatile bool gOHCITransferComplete = false;
bool OHCISendControlRequest(u8 FunctionAddress, u8 RequestType, u8 Request, u16 Value, u16 Index, u16 Count, void *DataBuffer, u16 MaxPacketSize) {
    gOHCI.RequestStorage->Endpoint.Control.MaxPacketSize   = MaxPacketSize;
    gOHCI.RequestStorage->Endpoint.Control.FunctionAddress = FunctionAddress;
    if (Count > 0 && DataBuffer) {
        MemSet(DataBuffer, 0, Count);
    }

    gOHCIRegisters->Control.ControlListEnable = 0;

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
    gOHCIRegisters->CommandStatus.ControlListFilled = 1;
    gOHCIRegisters->Control.ControlListEnable       = 1;

    gOHCITransferComplete = false;
    u32 Timeout           = 100;
    while (!gOHCITransferComplete && --Timeout) {
        SleepMS(1);
    }
    return Timeout != 0;
}

// #define OHCI_DEBUG_HID_INPUT

void HIDKeyboardReport(union HIDKeyboardReport *Report) {
#ifdef OHCI_DEBUG_HID_INPUT
    DebugLinef("HID Keyboard Report:");
    DebugLinef("  Modifiers:"
               " LCtrl=%u LShift=%u LAlt=%u LGUI=%u"
               " RCtrl=%u RShift=%u RAlt=%u RGUI=%u",
               Report->LeftCtrl,
               Report->LeftShift,
               Report->LeftAlt,
               Report->LeftGUI,
               Report->RightCtrl,
               Report->RightShift,
               Report->RightAlt,
               Report->RightGUI);

    DebugLinef("  Reserved: 0x%02X", Report->Reserved);

    DebugLinef("  KeyCodes:");
    for (int i = 0; i < 6; i++) {
        DebugLinef("    [%d] = 0x%02X", i, Report->KeyCode[i]);
    }

    DebugLinef("  Raw Value: 0x%016llX", (unsigned long long) Report->Value);
#endif

    struct InputKeyboardEvent Event = {0};
    Event.LeftCtrl                  = Report->LeftCtrl;
    Event.LeftShift                 = Report->LeftShift;
    Event.LeftAlt                   = Report->LeftAlt;
    Event.RightCtrl                 = Report->RightCtrl;
    Event.RightShift                = Report->RightShift;
    Event.RightAlt                  = Report->RightAlt;
    Event.RightGUI                  = Report->RightGUI;
    memcpy(Event.KeyCode, (const void *)Report->KeyCode, sizeof(Event.KeyCode));
    InputPushKeyboardEvent(&Event);
}

void HIDMouseReport(union HIDMouseReport *Report) {
#ifdef OHCI_DEBUG_HID_INPUT
    DebugLinef("HID Mouse Report:");
    DebugLinef("  Buttons: L=%u R=%u M=%u B3=%u B4=%u", Report->Left, Report->Right, Report->Middle, Report->Button3, Report->Button4);

    DebugLinef("  Movement: X=%d  Y=%d  Wheel=%d", Report->X, Report->Y, Report->Wheel);

    DebugLinef("  Raw Value: 0x%08X", Report->Value);
#endif
}

void OHCIHIDDeviceReportRecieved(struct OHCIDevice *Device) {
    if (Device->InterfaceClass != USB_INTERFACE_HID)
        return;
    if (!Device->Active)
        return;

    switch (Device->HID.HIDClass) {
    case KEYBOARD: {
        HIDKeyboardReport(&Device->HID.KeyboardReport);
        break;
    }
    case MOUSE: {
        HIDMouseReport(&Device->HID.MouseReport);
        break;
    }
    }
}

void OHCIInterruptHandler(struct CPUInterruptFrame *Frame) {
    DisableInterrupts();
    union OHCIInterruptRegister Status = gOHCIRegisters->InterruptStatus;

    if (Status.WritebackDoneHead) {
        u32 DoneQueueHead = gOHCI.HCCABase->DoneHead & 0xFFFFFFFE;

        gOHCI.HCCABase->DoneHead = 0;

        while (DoneQueueHead != 0) {
            struct OHCIGeneralTransferDescriptor *CompletedTD = (struct OHCIGeneralTransferDescriptor *) (usize) DoneQueueHead;

            u32 NextCompletedTD                 = CompletedTD->NextTD;
            CompletedTD->Control.ErrorCount     = 0;
            CompletedTD->Control.DelayInterrupt = 0;

            bool IsControl = false;
            if (CompletedTD >= &gOHCI.RequestStorage->TransferDescriptor[0] && CompletedTD < &gOHCI.RequestStorage->TransferDescriptor[64]) {
                gOHCITransferComplete = true;
                IsControl             = true;
            }

            if (!IsControl) {
                for (u32 i = 0; i < USB_MAX_DEVICES; i++) {
                    struct OHCIDevice *Dev = &gOHCI.Devices[i];
                    if (!Dev->Active || Dev->Interface->InterfaceClass != USB_INTERFACE_HID)
                        continue;

                    if ((usize) CompletedTD == (usize) Dev->HID.TransferDescriptor) {
                        if (CompletedTD->Control.ConditionCode == 0) {
                            OHCIHIDDeviceReportRecieved(Dev);
                        } else {
                            DebugLinef("HID Error CC: %x", CompletedTD->Control.ConditionCode);
                        }

                        CompletedTD->CurrentBufferPointer = (u32) (usize) Dev->HID.Data;
                        CompletedTD->BufferEnd = (u32) (usize) ((char *) Dev->HID.Data + Dev->HID.EndpointDescriptor->Control.MaxPacketSize - 1);
                        CompletedTD->NextTD    = (u32) (usize) Dev->HID.DummyTransferDescriptor;

                        u32 CurrentHead                                = Dev->HID.EndpointDescriptor->HeadPointer.Value;
                        Dev->HID.EndpointDescriptor->HeadPointer.Value = ((u32) ((usize) CompletedTD) & 0xFFFFFFF0) | (CurrentHead & 0x00000002);
                        break;
                    }
                }
            }

            DoneQueueHead = NextCompletedTD;
        }
    }

    gOHCIRegisters->InterruptStatus.Value = Status.Value;
    EnableInterrupts();
    OutByte(0x20, 0x20);
    OutByte(0xA0, 0x20);
}
