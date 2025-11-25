#pragma once
#include "elf_libc.c"
#include <string.h>

enum InputKeyboardScancodes {
    SCANCODE_KEY_NONE            = 0x00,
    SCANCODE_KEY_ERROR_ROLLOVER  = 0x01,
    SCANCODE_KEY_POST_FAIL       = 0x02,
    SCANCODE_KEY_ERROR_UNDEFINED = 0x03,

    SCANCODE_KEY_A = 0x04,
    SCANCODE_KEY_B = 0x05,
    SCANCODE_KEY_C = 0x06,
    SCANCODE_KEY_D = 0x07,
    SCANCODE_KEY_E = 0x08,
    SCANCODE_KEY_F = 0x09,
    SCANCODE_KEY_G = 0x0A,
    SCANCODE_KEY_H = 0x0B,
    SCANCODE_KEY_I = 0x0C,
    SCANCODE_KEY_J = 0x0D,
    SCANCODE_KEY_K = 0x0E,
    SCANCODE_KEY_L = 0x0F,
    SCANCODE_KEY_M = 0x10,
    SCANCODE_KEY_N = 0x11,
    SCANCODE_KEY_O = 0x12,
    SCANCODE_KEY_P = 0x13,
    SCANCODE_KEY_Q = 0x14,
    SCANCODE_KEY_R = 0x15,
    SCANCODE_KEY_S = 0x16,
    SCANCODE_KEY_T = 0x17,
    SCANCODE_KEY_U = 0x18,
    SCANCODE_KEY_V = 0x19,
    SCANCODE_KEY_W = 0x1A,
    SCANCODE_KEY_X = 0x1B,
    SCANCODE_KEY_Y = 0x1C,
    SCANCODE_KEY_Z = 0x1D,

    SCANCODE_KEY_1 = 0x1E,
    SCANCODE_KEY_2 = 0x1F,
    SCANCODE_KEY_3 = 0x20,
    SCANCODE_KEY_4 = 0x21,
    SCANCODE_KEY_5 = 0x22,
    SCANCODE_KEY_6 = 0x23,
    SCANCODE_KEY_7 = 0x24,
    SCANCODE_KEY_8 = 0x25,
    SCANCODE_KEY_9 = 0x26,
    SCANCODE_KEY_0 = 0x27,

    SCANCODE_KEY_ENTER     = 0x28,
    SCANCODE_KEY_ESCAPE    = 0x29,
    SCANCODE_KEY_BACKSPACE = 0x2A,
    SCANCODE_KEY_TAB       = 0x2B,
    SCANCODE_KEY_SPACE     = 0x2C,

    SCANCODE_KEY_MINUS      = 0x2D, // -
    SCANCODE_KEY_EQUAL      = 0x2E, // =
    SCANCODE_KEY_LEFTBRACE  = 0x2F, // [
    SCANCODE_KEY_RIGHTBRACE = 0x30, // ]
    SCANCODE_KEY_BACKSLASH  = 0x31, // (\\)
    SCANCODE_KEY_NONUS_HASH = 0x32, //
    SCANCODE_KEY_SEMICOLON  = 0x33, // ;
    SCANCODE_KEY_APOSTROPHE = 0x34, // '
    SCANCODE_KEY_GRAVE      = 0x35, // `
    SCANCODE_KEY_COMMA      = 0x36, // ,
    SCANCODE_KEY_DOT        = 0x37, // .
    SCANCODE_KEY_SLASH      = 0x38, // /

    SCANCODE_KEY_CAPSLOCK = 0x39,
    SCANCODE_KEY_F1       = 0x3A,
    SCANCODE_KEY_F2       = 0x3B,
    SCANCODE_KEY_F3       = 0x3C,
    SCANCODE_KEY_F4       = 0x3D,
    SCANCODE_KEY_F5       = 0x3E,
    SCANCODE_KEY_F6       = 0x3F,
    SCANCODE_KEY_F7       = 0x40,
    SCANCODE_KEY_F8       = 0x41,
    SCANCODE_KEY_F9       = 0x42,
    SCANCODE_KEY_F10      = 0x43,
    SCANCODE_KEY_F11      = 0x44,
    SCANCODE_KEY_F12      = 0x45,

    SCANCODE_KEY_PRINTSCREEN = 0x46,
    SCANCODE_KEY_SCROLLLOCK  = 0x47,
    SCANCODE_KEY_PAUSE       = 0x48,
    SCANCODE_KEY_INSERT      = 0x49,
    SCANCODE_KEY_HOME        = 0x4A,
    SCANCODE_KEY_PAGEUP      = 0x4B,
    SCANCODE_KEY_DELETE      = 0x4C,
    SCANCODE_KEY_END         = 0x4D,
    SCANCODE_KEY_PAGEDOWN    = 0x4E,
    SCANCODE_KEY_RIGHT       = 0x4F,
    SCANCODE_KEY_LEFT        = 0x50,
    SCANCODE_KEY_DOWN        = 0x51,
    SCANCODE_KEY_UP          = 0x52,

    SCANCODE_KEY_NUMLOCK     = 0x53,
    SCANCODE_KEY_KP_DIVIDE   = 0x54,
    SCANCODE_KEY_KP_MULTIPLY = 0x55,
    SCANCODE_KEY_KP_MINUS    = 0x56,
    SCANCODE_KEY_KP_PLUS     = 0x57,
    SCANCODE_KEY_KP_ENTER    = 0x58,
    SCANCODE_KEY_KP_1        = 0x59,
    SCANCODE_KEY_KP_2        = 0x5A,
    SCANCODE_KEY_KP_3        = 0x5B,
    SCANCODE_KEY_KP_4        = 0x5C,
    SCANCODE_KEY_KP_5        = 0x5D,
    SCANCODE_KEY_KP_6        = 0x5E,
    SCANCODE_KEY_KP_7        = 0x5F,
    SCANCODE_KEY_KP_8        = 0x60,
    SCANCODE_KEY_KP_9        = 0x61,
    SCANCODE_KEY_KP_0        = 0x62,
    SCANCODE_KEY_KP_DOT      = 0x63,

    SCANCODE_KEY_NONUS_BACKSLASH = 0x64,

    SCANCODE_KEY_APPLICATION = 0x65,
    SCANCODE_KEY_POWER       = 0x66,
    SCANCODE_KEY_KP_EQUAL    = 0x67,

    SCANCODE_KEY_F13 = 0x68,
    SCANCODE_KEY_F14 = 0x69,
    SCANCODE_KEY_F15 = 0x6A,
    SCANCODE_KEY_F16 = 0x6B,
    SCANCODE_KEY_F17 = 0x6C,
    SCANCODE_KEY_F18 = 0x6D,
    SCANCODE_KEY_F19 = 0x6E,
    SCANCODE_KEY_F20 = 0x6F,
    SCANCODE_KEY_F21 = 0x70,
    SCANCODE_KEY_F22 = 0x71,
    SCANCODE_KEY_F23 = 0x72,
    SCANCODE_KEY_F24 = 0x73,

    SCANCODE_KEY_CONTROL = 0xE0,
    SCANCODE_KEY_SHIFT   = 0xE1,
    SCANCODE_KEY_ALT     = 0xE2,

    SCANCODE_KEY_COUNT,
};

#define SCANCODES_PER_EVENT 6

struct InputKeyboardEvent {
    bool LeftCtrl;
    bool LeftShift;
    bool LeftAlt;
    bool RightCtrl;
    bool RightShift;
    bool RightAlt;
    bool RightGUI;
    u8   KeyCode[SCANCODES_PER_EVENT];
};

union InputMouseEvent {
    bool ButtonLeft;
    bool ButtonRight;
    bool ButtonMiddle;
    bool Button3;
    bool Button4;

    i8 DeltaX;
    i8 Deltay;
    i8 DeltaWheel;
};

static bool gInputKeysDown[256] = {0};
static bool gInputCapsLock      = false;

#define INPUT_EVENT_RINGBUFFER_SIZE 16

struct InputCharacterEvent {
    usize TickCount;
    u8    ScanCode;
    char  ASCII;

    bool Shift, Alt, Control;
};

static struct InputCharacterEvent gInputKeyPressedQueue[INPUT_EVENT_RINGBUFFER_SIZE]  = {0};
static struct InputCharacterEvent gInputKeyReleasedQueue[INPUT_EVENT_RINGBUFFER_SIZE] = {0};
static usize                      gInputKeyPressedIndex                               = 0;
static usize                      gInputKeyReleasedIndex                              = 0;

char InputScancodeToASCII(enum InputKeyboardScancodes Code);
void InputPushKeyboardEvent(struct InputKeyboardEvent *Event) {
    // PrintLine("event");
    bool OldKeysDown[256] = {0};
    memcpy(OldKeysDown, gInputKeysDown, sizeof(OldKeysDown));

    for (u8 ScanCode = 0; ScanCode < SCANCODE_KEY_COUNT; ++ScanCode) {
        gInputKeysDown[ScanCode] = false;
    }

    for (u8 InputScanCodeIndex = 0; InputScanCodeIndex < SCANCODES_PER_EVENT; ++InputScanCodeIndex) {
        u8 PressedScanCode = Event->KeyCode[InputScanCodeIndex];
        if (PressedScanCode == SCANCODE_KEY_NONE) {
            break;
        }
        gInputKeysDown[PressedScanCode] = true;
    }
    gInputKeysDown[SCANCODE_KEY_SHIFT]   = Event->LeftShift || Event->RightShift;
    gInputKeysDown[SCANCODE_KEY_ALT]     = Event->LeftAlt || Event->RightAlt;
    gInputKeysDown[SCANCODE_KEY_CONTROL] = Event->LeftCtrl || Event->RightCtrl;

    usize Tick = TickCount;
    for (u8 ScanCode = 0; ScanCode < SCANCODE_KEY_COUNT; ++ScanCode) {
        struct InputCharacterEvent Event = {
            .TickCount = Tick,
            .ScanCode  = ScanCode,
            .ASCII     = InputScancodeToASCII(ScanCode),
            .Shift     = gInputKeysDown[SCANCODE_KEY_SHIFT],
            .Control   = gInputKeysDown[SCANCODE_KEY_CONTROL],
            .Alt       = gInputKeysDown[SCANCODE_KEY_ALT],
        };

        if ((gInputKeysDown[ScanCode] == true && OldKeysDown[ScanCode] == false)) {
            gInputKeyPressedQueue[gInputKeyPressedIndex] = Event;
            gInputKeyPressedIndex                        = (gInputKeyPressedIndex + 1) % INPUT_EVENT_RINGBUFFER_SIZE;
            if (ScanCode == SCANCODE_KEY_CAPSLOCK) {
                gInputCapsLock = !gInputCapsLock;
            }
        }
        if (gInputKeysDown[ScanCode] == false && OldKeysDown[ScanCode] == true) {
            gInputKeyReleasedQueue[gInputKeyReleasedIndex] = Event;
            gInputKeyReleasedIndex                         = (gInputKeyReleasedIndex + 1) % INPUT_EVENT_RINGBUFFER_SIZE;
        }
    }

    if (gInputKeysDown[SCANCODE_KEY_CONTROL] && gInputKeysDown[SCANCODE_KEY_ALT] && gInputKeysDown[SCANCODE_KEY_DELETE]) {
        TriggerTripleFault();
    }
}

void InputPushMouseEvent(struct InputKeyboardEvent *Event) {}

bool InputScancodeIsASCII(enum InputKeyboardScancodes code) {
    switch (code) {
    case SCANCODE_KEY_A ... SCANCODE_KEY_Z:
    case SCANCODE_KEY_1 ... SCANCODE_KEY_0:
    case SCANCODE_KEY_MINUS:
    case SCANCODE_KEY_EQUAL:
    case SCANCODE_KEY_LEFTBRACE:
    case SCANCODE_KEY_RIGHTBRACE:
    case SCANCODE_KEY_BACKSLASH:
    case SCANCODE_KEY_SEMICOLON:
    case SCANCODE_KEY_APOSTROPHE:
    case SCANCODE_KEY_GRAVE:
    case SCANCODE_KEY_COMMA:
    case SCANCODE_KEY_DOT:
    case SCANCODE_KEY_SLASH:
    case SCANCODE_KEY_SPACE:
        return true;
    default:
        return false;
    }
}

char InputScancodeToASCII(enum InputKeyboardScancodes Code) {
    bool Shift = gInputKeysDown[SCANCODE_KEY_SHIFT];
    char Output = '\0';
    switch (Code) {
    case SCANCODE_KEY_A:
        Output = 'a';
        break;
    case SCANCODE_KEY_B:
        Output = 'b';
        break;
    case SCANCODE_KEY_C:
        Output = 'c';
        break;
    case SCANCODE_KEY_D:
        Output = 'd';
        break;
    case SCANCODE_KEY_E:
        Output = 'e';
        break;
    case SCANCODE_KEY_F:
        Output = 'f';
        break;
    case SCANCODE_KEY_G:
        Output = 'g';
        break;
    case SCANCODE_KEY_H:
        Output = 'h';
        break;
    case SCANCODE_KEY_I:
        Output = 'i';
        break;
    case SCANCODE_KEY_J:
        Output = 'j';
        break;
    case SCANCODE_KEY_K:
        Output = 'k';
        break;
    case SCANCODE_KEY_L:
        Output = 'l';
        break;
    case SCANCODE_KEY_M:
        Output = 'm';
        break;
    case SCANCODE_KEY_N:
        Output = 'n';
        break;
    case SCANCODE_KEY_O:
        Output = 'o';
        break;
    case SCANCODE_KEY_P:
        Output = 'p';
        break;
    case SCANCODE_KEY_Q:
        Output = 'q';
        break;
    case SCANCODE_KEY_R:
        Output = 'r';
        break;
    case SCANCODE_KEY_S:
        Output = 's';
        break;
    case SCANCODE_KEY_T:
        Output = 't';
        break;
    case SCANCODE_KEY_U:
        Output = 'u';
        break;
    case SCANCODE_KEY_V:
        Output = 'v';
        break;
    case SCANCODE_KEY_W:
        Output = 'w';
        break;
    case SCANCODE_KEY_X:
        Output = 'x';
        break;
    case SCANCODE_KEY_Y:
        Output = 'y';
        break;
    case SCANCODE_KEY_Z:
        Output = 'z';
        break;

    case SCANCODE_KEY_1:
        Output = Shift ? '!' : '1';
        break;
    case SCANCODE_KEY_2:
        Output = Shift ? '@' : '2';
        break;
    case SCANCODE_KEY_3:
        Output = Shift ? '#' : '3';
        break;
    case SCANCODE_KEY_4:
        Output = Shift ? '$' : '4';
        break;
    case SCANCODE_KEY_5:
        Output = Shift ? '%' : '5';
        break;
    case SCANCODE_KEY_6:
        Output = Shift ? '^' : '6';
        break;
    case SCANCODE_KEY_7:
        Output = Shift ? '&' : '7';
        break;
    case SCANCODE_KEY_8:
        Output = Shift ? '*' : '8';
        break;
    case SCANCODE_KEY_9:
        Output = Shift ? '(' : '9';
        break;
    case SCANCODE_KEY_0:
        Output = Shift ? ')' : '0';
        break;

    case SCANCODE_KEY_MINUS:
        Output = Shift ? '_' : '-';
        break;
    case SCANCODE_KEY_EQUAL:
        Output = Shift ? '+' : '=';
        break;
    case SCANCODE_KEY_LEFTBRACE:
        Output = Shift ? '{' : '[';
        break;
    case SCANCODE_KEY_RIGHTBRACE:
        Output = Shift ? '}' : ']';
        break;
    case SCANCODE_KEY_BACKSLASH:
        Output = Shift ? '|' : '\\';
        break;
    case SCANCODE_KEY_SEMICOLON:
        Output = Shift ? ':' : ';';
        break;
    case SCANCODE_KEY_APOSTROPHE:
        Output = Shift ? '"' : '\'';
        break;
    case SCANCODE_KEY_GRAVE:
        Output = Shift ? '~' : '`';
        break;
    case SCANCODE_KEY_COMMA:
        Output = Shift ? '<' : ',';
        break;
    case SCANCODE_KEY_DOT:
        Output = Shift ? '>' : '.';
        break;
    case SCANCODE_KEY_SLASH:
        Output = Shift ? '?' : '/';
        break;
    case SCANCODE_KEY_SPACE:
        Output = ' ';
        break;

    case SCANCODE_KEY_ENTER:
        Output = '\n';
        break;
    case SCANCODE_KEY_TAB:
        Output = '\t';
        break;
    case SCANCODE_KEY_BACKSPACE:
        Output = '\b';
        break;
    default:
        return '\0';
    }

    // if it's a letter and shift is active, convert to uppercase
    if (gInputCapsLock) {
        Shift = !Shift;
    }

    if (Shift && Output >= 'a' && Output <= 'z') {
        Output = Output - 'a' + 'A';
    }

    return Output;
}

struct InputCharacterEventSubscriber {
    usize                      LastTickCount;
    struct InputCharacterEvent LastEvent;
};

bool InputPollCharacterEvent(struct InputCharacterEventSubscriber *Subscriber, struct InputCharacterEvent *EventQueue) {
    // search through event queue, if there are no events NEWER (or no NEW events newer or the same age), return error
    usize SmallestTickCount = 0xFFFFFFFFFFFFFFFF;
    usize FoundIndex        = 0;
    bool  Found             = false;
    for (usize Index = 0; Index < INPUT_EVENT_RINGBUFFER_SIZE; ++Index) {
        struct InputCharacterEvent Event = EventQueue[Index];
        if (Event.ScanCode == SCANCODE_KEY_NONE)
            continue;
        // PrintLinef("%zu EVENT %zu %zu %zu", Index, Event.TickCount, Subscriber->LastTickCount, gInputKeyPressedIndex);

        // 1. is it eligible
        bool Eligible = Event.TickCount > Subscriber->LastTickCount;
        if (!Eligible)
            continue;

        // 2. is is the smallest tickcount we've seen so far
        bool Smallest = Event.TickCount <= SmallestTickCount;
        if (!Smallest)
            continue;

        FoundIndex        = Index;
        SmallestTickCount = Event.TickCount;
        Found             = true;
    }
    if (!Found) {
        return false;
    }

    struct InputCharacterEvent *Event = &EventQueue[FoundIndex];
    Subscriber->LastTickCount         = SmallestTickCount;
    Subscriber->LastEvent             = *Event;

    Printf("");
    return true;
}

void InputNextCharacterEvent(struct InputCharacterEventSubscriber *Subscriber, struct InputCharacterEvent *EventQueue) {
    while (!InputPollCharacterEvent(Subscriber, EventQueue)) {
        SleepMS(1);
    }
}

char InputNextASCIICharacterEvent(struct InputCharacterEventSubscriber *Subscriber, struct InputCharacterEvent *EventQueue) {
    while (true) {
        InputNextCharacterEvent(Subscriber, EventQueue);
        if (Subscriber->LastEvent.ASCII) {
            return Subscriber->LastEvent.ASCII;
        }
    }
}
