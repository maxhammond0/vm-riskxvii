#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define OFFSET 4

void handle_file(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("File could not be opened\n");
        printf("exiting...\n");
        exit(2);
    }
    char buff[OFFSET] = "";
    int read, address = 0;

    while ((read = fread(buff, 1, sizeof buff, fp)) > 0) {
        printf("%03x ", address);
        address += OFFSET;

        uint8_t op = 0u;
        for (int i = OFFSET; i >= 0; i--) {    /* print hex values */
            if (i < read) {
                op |= (unsigned char)buff[i] << i*8;
                printf("%02hhx", (unsigned char)buff[i]);
            }
        }
        printf(" %x", op);

        putchar ('\n'); /* use putchar to output single character */
    }

    fclose(fp);
}


int main (int argc, char const *argv[]) 
{
    if (argc != 2) {
        printf("Incorrect argument count\n");
        printf("Should be ./vm_riskxvii <file_path>\n");
        printf("exiting...\n");
        return 1;
    }

    handle_file(argv[1]);

    return 0;
}
