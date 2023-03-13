#include <stdio.h>
#include <stdlib.h>

#define OFFSET 4

int main (int argc, char const *argv[]) 
{
    char buff[OFFSET] = "";
    int read, address = 0, i;
    if (argc != 2) {
        printf("Incorrect usage\n");
        return 1;
    }
    FILE *fp = fopen (argv[1], "rb");

    if (!fp) {
        perror ("fopen");
        return 2;
    }

    while ((read = fread(buff, 1, sizeof buff, fp)) > 0) {
        printf("%08x ", address);
        address += OFFSET;

        for (i = 0; i < OFFSET; i++)    /* print hex values */
            if (i >= read || buff[i] == 0)
                printf("   ");
            else
                printf("%02hhx ", (unsigned char)buff[i]);

        putchar ('\n'); /* use putchar to output single character */
    }

    fclose (fp);

    return 0;
}
