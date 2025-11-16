#include "elf_libc.c"

i32 main() {
    u64 CorrectNumber = RandomInt64() % 100;
    u64 Guesses       = 0;

    PrintLine("Guess a number between 1 and 100...");
    for (;;) {
        char  Buffer[256] = {0};
        usize Length      = ReadLine(Buffer);

        u64 Number = StringToInt64(Buffer);
        if (Number == CorrectNumber) {
            Print("Correct! Took you ");
            PrintNumber(Guesses);
            PrintLine(" guesses!");
            break;
        } else if (Number > CorrectNumber) {
            PrintLine("Too high!");
        } else {
            PrintLine("Too low!");
        }
        ++Guesses;
    }
    return 0;
}
