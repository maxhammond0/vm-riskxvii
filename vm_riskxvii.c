#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

#define OFFSET 4


int main (int argc, char const *argv[]) 
{
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


    fclose(fp);

    return 0;
}
