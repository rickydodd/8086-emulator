#include <stdio.h>

int main(int argc, char *argv[])
{
    unsigned char buffer[2];

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

    if (in_file != NULL) {
        fclose(in_file);
    }
    return 0;
}

