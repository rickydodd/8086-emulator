#include <stdio.h>
#include <string.h>

// opcodes
#define MOV_OPCODE 0b100010

// modes
#define REGISTER_TO_REGISTER_MODE 0b11

// directions
#define REG_IS_SOURCE 0
#define REG_IS_DESTINATION 1

// bitmasks
#define REG_BITMASK 0x38
#define RM_BITMASK 0x7
#define W_BITMASK 1

typedef unsigned short u16;

int main(int argc, char *argv[])
{
    u16 reg; // stores a register operand
    u16 rm; // stores a register operand
    u16 word; // 0 if instr. operates on byte data, 1 if word data
    u16 mode; // 0b11 if register mode, else memory mode with displacement length
    u16 direction; // 0 if source is specified in REG field, 1 if destination is specified in REG field
    unsigned char buffer[2];
    char *regs[8][2] = { // regs[register value][word]
        {"al", "ax"},
        {"cl", "cx"},
        {"dl", "dx"},
        {"bl", "bx"},
        {"ah", "sp"},
        {"ch", "bp"},
        {"dh", "si"},
        {"bh", "di"}
    };

    if (argc == 1) {
        printf("Not enough arguments. Usage: 8086emu filename -d\n");
        return 1;
    } else if (argc > 3) {
        printf("Too many arguments. Usage: 8086emu filename -d\n");
        return 1;
    }

    // open bin file
    printf("Attempting to open file: %s\n", argv[1]);
    FILE *in_file = fopen(argv[1], "r");
    if (in_file == NULL) {
        printf("File could not be opened.\n");
        return 1;
    }
    printf("File successfully opened.\n");

    // disassembly mode
    if (argv[2] != NULL && strcmp(argv[2], "-d") == 0) {
        printf("; %s, x86-16 (8086) disassembly\n", argv[1]);
        printf("bits 16\n");
        while (fread(buffer, 2, 1, in_file) != 0) {
            if (buffer[0] >> 2 == MOV_OPCODE) {
                reg = (buffer[1] & REG_BITMASK) >> 3;
                rm = buffer[1] & RM_BITMASK;
                word = buffer[0] & W_BITMASK;
                mode = buffer[1] >> 6;
                direction = (buffer[0] & 2) >> 1;

                printf("mov ");

                // mov destination
                switch (direction) {
                    case REG_IS_SOURCE:
                        printf("%s", regs[rm][word]);
                        break;
                    case REG_IS_DESTINATION:
                        printf("%s", regs[reg][word]);
                        break;
                }

                printf(", ");

                // mov source
                if (mode == REGISTER_TO_REGISTER_MODE) {
                    switch (direction) {
                        case REG_IS_SOURCE:
                            printf("%s\n", regs[reg][word]);
                            break;
                        case REG_IS_DESTINATION:
                            printf("%s\n", regs[rm][word]);
                            break;
                    }
                }
            }
        }
    }

    if (in_file != NULL) {
        fclose(in_file);
    }
    return 0;
}

