#include <stdio.h>
#include <stdlib.h>

int read_file(const char* filename) {

    FILE *file = fopen(filename, "rb");
    char ch;

    if (file == NULL) {
        printf("File could not be opened\nExiting...\n");
        exit(2);
    }
    do {
        ch = fgetc(file);
        printf("%x", ch);
 
        // Checking if character is not EOF.
        // If it is EOF stop reading.
    } while (ch != EOF);
    printf("\n");

    fclose(file);

    return 0;
}

int main(int argc, char **argv) {

    if (argc != 2) {
        printf("Wrong number of arguments\n");
        printf("First argument should be path to binary file\n");
        printf("Exiting...\n");
        return 1;
    }

    char *file = argv[1];
    read_file(file);

    return 0;
}
