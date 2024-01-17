#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define MEMORY_SIZE 65536 // x86-16 has 64 KB of main memory

// opcodes
#define REGISTER_TO_REGISTER_MOV_OPCODE 0b100010
#define IMMEDIATE_TO_REGISTER_OPCODE 0b1011

// modes
#define MEMORY_MODE 0b00
#define MEMORY_MODE_8BIT_DISPLACEMENT 0b01
#define MEMORY_MODE_16BIT_DISPLACEMENT 0b10
#define REGISTER_MODE 0b11

// directions
#define REG_IS_SOURCE 0
#define REG_IS_DESTINATION 1

// convert two 8-bit values, LO and HI, to a signed 16-bit integer
#define LO_HI_TO_I16(LO, HI) (i16)(((i16)HI << 8) + LO)

typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;

int main(int argc, char *argv[])
{
    u16 tmp;
    u8 *memory = (u8 *) malloc(sizeof(u8) * MEMORY_SIZE); // main memory
    u8 reg; // stores a register operand
    u8 rm; // stores a register operand
    u8 word; // 0 if instr. operates on byte data, 1 if word data
    u8 mode; // 0b11 if register mode, else memory mode with displacement length
    u8 direction; // 0 if source is specified in REG field, 1 if destination is specified in REG field
    unsigned char buffer[4];
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
    char *effective_addr[8] = { // effective address calculation
        "bx + si",
        "bx + di",
        "bp + si",
        "bp + di",
        "si",
        "di",
        "bp",
        "bx"
    };

    if (memory == NULL) {
        printf("Failed to allocate memory.\n");
        return 1;
    }

    if (argc == 1) {
        printf("Not enough arguments. Usage: 8086emu filename -d\n");
        return 1;
    } else if (argc > 3) {
        printf("Too many arguments. Usage: 8086emu filename -d\n");
        return 1;
    }

    // open bin file
    FILE *in_file = fopen(argv[1], "r");
    if (in_file == NULL) {
        printf("File could not be opened.\n");
        return 1;
    }

    // disassembly mode
    if (argv[2] != NULL && strcmp(argv[2], "-d") == 0) {
        printf("; %s, x86-16 (8086) disassembly\n", argv[1]);
        printf("bits 16\n");
        while (fread(buffer, 2, 1, in_file) != 0) {
            if (buffer[0] >> 2 == REGISTER_TO_REGISTER_MOV_OPCODE) {
                reg = (buffer[1] & 0x38) >> 3;
                rm = buffer[1] & 0x7;
                word = buffer[0] & 1;
                mode = buffer[1] >> 6;
                direction = (buffer[0] & 2) >> 1;

                printf("mov ");

                // mov destination
                if (direction == REG_IS_SOURCE) {
                    switch (mode) {
                        case MEMORY_MODE:
                            printf("[%s]", effective_addr[rm]);
                            break;
                        case MEMORY_MODE_8BIT_DISPLACEMENT:
                            fread(buffer, 1, 1, in_file); // read 8-bit displacement byte

                            printf("[%s", effective_addr[rm]);

                            // if the 8-bit displacement value is not zero, then include it in
                            // disassembly. if it is zero, then ignore it.
                            if (buffer[0] != 0) {
                                printf(" + %d]", buffer[0]);
                            } else {
                                printf("]");
                            }
                            break;
                        case MEMORY_MODE_16BIT_DISPLACEMENT:
                            fread(buffer, 2, 1, in_file); // read LO and HI bytes

                            printf("[%s", effective_addr[rm]);

                            // if the 16-bit displacement value is not zero, then include it in
                            // disassembly. if it is zero, then ignore it.
                            if (buffer[0] != 0 && buffer[1] != 0) {
                                printf(" + %d]", LO_HI_TO_I16(buffer[0], buffer[1]));
                            } else {
                                printf("]");
                            }
                            break;
                        case REGISTER_MODE:
                            printf("%s", regs[rm][word]);
                    }
                } else if (direction == REG_IS_DESTINATION) {
                    printf("%s", regs[reg][word]);
                }

                printf(", ");

                // mov source
                if (direction == REG_IS_SOURCE) {
                    printf("%s\n", regs[reg][word]);
                } else if (direction == REG_IS_DESTINATION) {
                    switch (mode) {
                        case MEMORY_MODE:
                            printf("[%s]\n", effective_addr[rm]);
                            break;
                        case MEMORY_MODE_8BIT_DISPLACEMENT:
                            fread(buffer, 1, 1, in_file); // read 8-bit displacement byte

                            printf("[%s", effective_addr[rm]);

                            // if the 8-bit displacement value is not zero, then include it in
                            // disassembly. if it is zero, then ignore it.
                            if (buffer[0] != 0) {
                                printf(" + %d]\n", buffer[0]);
                            } else {
                                printf("]\n");
                            }
                            break;
                        case MEMORY_MODE_16BIT_DISPLACEMENT:
                            fread(buffer, 2, 1, in_file); // read LO and HI bytes, respectively

                            printf("[%s", effective_addr[rm]);

                            // if the 16-bit displacement value is not zero, then include it in
                            // disassembly. if it is zero, then ignore it.
                            if (LO_HI_TO_I16(buffer[0], buffer[1]) != 0) {
                                printf(" + %d]\n", LO_HI_TO_I16(buffer[0], buffer[1]));
                            } else {
                                printf("]\n");
                            }
                            break;
                        case REGISTER_MODE:
                            printf("%s\n", regs[rm][word]);
                    }
                }
            } else if (buffer[0] >> 4 == IMMEDIATE_TO_REGISTER_OPCODE) {
                reg = buffer[0] & 0x7; // reg field
                word = (buffer[0] & 0x8) >> 3; // w field

                if (word == 0) { // if word = 0, then 8-bit immediate
                    printf("mov %s, %d\n", regs[reg][word], (i8)buffer[1]);
                } else { // else, 16-bit immediate
                    // write the 8-bit data byte to tmp and read in the second data byte,
                    // because w = 1
                    tmp = buffer[1];
                    fread(buffer, 1, 1, in_file);

                    printf("mov %s, %d\n", regs[reg][word], LO_HI_TO_I16(tmp, buffer[0]));
                }
            }
        }
    }

    if (in_file != NULL) {
        fclose(in_file);
    }
    free(memory);
    return 0;
}

