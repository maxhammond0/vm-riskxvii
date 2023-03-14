#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OFFSET 4


int main (int argc, char const *argv[]) 
{
    char buff[OFFSET] = "";
    int read, address = 0, i;
    if (argc != 2) {
        printf("Incorrect argument count\n");
        printf("Should be ./vm_riskxvii <file_path>\n");
        printf("exiting...\n");
        return 1;
    }
    FILE *fp = fopen (argv[1], "rb");

    if (!fp) {
        printf("File could not be opened\n");
        printf("exiting...\n");
        return 2;
    }
    // char instruction[9];
    // instruction[8] = '\0';
    // int j;
    // int j = 0;
    // while ((read = fread(buff, 1, sizeof buff, fp)) > 0) {
    //     j++;
    // }

    // hex string
    // char res[9];

    while ((read = fread(buff, 1, sizeof buff, fp)) > 0) {
        // Display byte number
        printf("%03x: ", address);
        address += OFFSET;

        // Print hex values
        for (i = OFFSET; i >= 0; i--) {
            if (i >= read ) {
                printf(" ");
            }
            else {
                // instruction[i] = (char)buff[i];

                // printf("%d", buff[i]);
                printf(" %02x ", (unsigned char)buff[i]);
            }

        }
        printf("\n");
        // printf("Instruction: %s\n", instruction);
    }

    fclose(fp);

    return 0;
}
