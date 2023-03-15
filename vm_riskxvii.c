#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

void convert_hex_to_bin(uint32_t hex) {
    // char binary[33] = {0};
}

int main( int argc, char *argv[])
{
    int fd;
    int retval;
    int each = 0;
    unsigned char buf[16] = {0};
    unsigned char opcode[4] = {0};
    unsigned char arg1[4] = {0};
    unsigned char arg2[4] = {0};
    unsigned char arg3[4] = {0};

    // Wrong number of cmd line arguments
    if ( argc != 2) {
        printf("Wrong number of command line arguments\n");
        printf("Exiting...\n");
        return 1;
    }

    // File can't be read
    if ((fd = open(argv[1], O_RDONLY)) < 0) {
        printf("Could not open file\n");
        printf("Exiting...\n");
        return 2;
    }

    // Read until EOF
    while ( ( retval = read ( fd, &buf, 4)) > 0) {
        // Read 4 bytes at a time
        if ( retval == 4) {
            for ( each = 0; each < 4; each++) {
                arg3[each] = buf[each];
                arg2[each] = buf[each + 1];
                arg1[each] = buf[each + 2];
                opcode[each] = buf[each + 3];
            }
            // printf("%02x%02x%02x%02x", (unsigned int)*opcode,
            //        (unsigned int)*arg1,
            //        (unsigned int)*arg2,
            //        (unsigned int)*arg3);
            uint32_t op = 0u;
            op |= (unsigned int)*opcode << 24;      // 0xAA000000
            op |= (unsigned int)*arg1 << 16;      // 0xaaBB0000
            op |= (unsigned int)*arg2 << 8;       // 0xaabbCC00
            op |= (unsigned int)*arg3;            // 0xaabbccDD
            printf("%08x\n", op);
            // convert_hex_to_bin(op);
        }
    }

    close(fd);
    return 0;
}
