#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

typedef uint32_t INSTRUCTION;

void read_file(char *filepath, INSTRUCTION *instructions) {
    // Reads file and loads instructions into the instructions array
    int fd;
    int retval;
    int each = 0;
    unsigned char buf[16] = {0};
    unsigned char opcode[4] = {0};
    unsigned char arg1[4] = {0};
    unsigned char arg2[4] = {0};
    unsigned char arg3[4] = {0};


    // File can't be read
    if ((fd = open(filepath, O_RDONLY)) < 0) {
        printf("Could not open file\n");
        printf("Exiting...\n");
        exit(2);
    }

    int pc = 0;
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

            INSTRUCTION op = 0u;
            op |= (unsigned int)*opcode << 24;      // 0xAA000000
            op |= (unsigned int)*arg1 << 16;      // 0xaaBB0000
            op |= (unsigned int)*arg2 << 8;       // 0xaabbCC00
            op |= (unsigned int)*arg3;            // 0xaabbccDD
            instructions[pc] = op;
            pc++;
        }
    }

}

void mask(int i, int j) {
    // Return 
}

void get_op() {

}

void get_func3() {

}

void get_func7() {

}

void process_instruction() {

}

int main( int argc, char *argv[]) {

    // Wrong number of cmd line arguments
    if ( argc != 2) {
        printf("Wrong number of command line arguments\n");
        printf("Exiting...\n");
        return 1;
    }

    INSTRUCTION instructions[1024] = { 0 };

    read_file(argv[1], instructions);

    // Print out all instructions (for debugging)
    // for (int i = 0; i < 1024; i++) {
    //     printf("%08x\n", instructions[i]);
    // }

    return 0;
}
