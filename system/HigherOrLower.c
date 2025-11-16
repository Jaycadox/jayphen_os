#include "elf_libc.c"

i32 main() {
    u64 CorrectNumber = RandomInt64() % 100;
    u64 Guesses       = 0;

    UEFIPrintLine("Guess a number between 1 and 100...");
    for (;;) {
        char  Buffer[256] = {0};
        usize Length      = UEFIReadLine(Buffer);

        u64 Number = StringToInt64(Buffer);
        if (Number == CorrectNumber) {
            UEFIPrintLinef("Correct! Took you %ld guesses!", Guesses);
            break;
        } else if (Number > CorrectNumber) {
            UEFIPrintLine("Too high!");
        } else {
            UEFIPrintLine("Too low!");
        }
        ++Guesses;
    }
    return 0;
}
