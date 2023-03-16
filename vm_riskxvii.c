#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <limits.h>

#define R 51
#define I 19
#define S 35
#define SB 99
#define U 55
#define UJ 111

typedef uint32_t INSTRUCTION;

const int R0 = 0;
int R1 = 0;
int R2 = 0;
int R3 = 0;
int R4 = 0;
int R5 = 0;
int R6 = 0;
int R7 = 0;
int R8 = 0;
int R9 = 0;
int R10 = 0;
int R11 = 0;
int R12 = 0;
int R13 = 0;
int R14 = 0;
int R15 = 0;
int R16 = 0;
int R17 = 0;
int R18 = 0;
int R19 = 0;
int R20 = 0;
int R21 = 0;
int R22 = 0;
int R23 = 0;
int R24 = 0;
int R25 = 0;
int R26 = 0;
int R27 = 0;
int R28 = 0;
int R29 = 0;
int R30 = 0;
int R31 = 0;

int gpregisters[31] = {0};

void print_binary(unsigned int number) {
    if (number >> 1) {
        print_binary(number >> 1);
    }
    putc((number & 1) ? '1' : '0', stdout);
}

void read_file(char *filepath, INSTRUCTION *instructions) {
    // Reads file and loads instructions into the instructions array
    int fd;
    int retval;
    int each = 0;
    unsigned char buf[16] = {0};
    unsigned char byte1[4] = {0};
    unsigned char byte2[4] = {0};
    unsigned char byte3[4] = {0};
    unsigned char byte4[4] = {0};


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
                // Read in little endian byte at a time
                byte4[each] = buf[each];
                byte3[each] = buf[each + 1];
                byte2[each] = buf[each + 2];
                byte1[each] = buf[each + 3];
            }

            // Spooky bit operations, get the little endianness of the
            // 32bit instruction into a 32bit unsigned integer variable
            INSTRUCTION op = 0u;
            op |= (unsigned int)*byte1 << 24;      // 0xAA000000
            op |= (unsigned int)*byte2 << 16;      // 0xaaBB0000
            op |= (unsigned int)*byte3 << 8;       // 0xaabbCC00
            op |= (unsigned int)*byte4;            // 0xaabbccDD
            instructions[pc] = op;
            pc++;
        }
    }

}

unsigned int mask(INSTRUCTION n, int i, int j) {
    // Return the ith to the jth bits of an INSTRUCTION
    // int p = j - i;
    int p = j - i + 1;
    i++;
    return (((1 << p) - 1) & (n >> (i - 1)));
}

void add(INSTRUCTION instruction) {
    // Type: R
    // opcode: 011011
    // func3: 000
    // func7: 0000000

}

void addi(INSTRUCTION instruction) {
    // Type: I
    // opcode: 001011
    // func3: 000
    unsigned int rd = mask(instruction, 7, 11);
    int rs1 = mask(instruction, 15, 19);
    int immi = mask(instruction, 20, 31);
    // printf("immi: ");
    // print_binary(immi);
    // printf("\n");
    // printf("%d, %d, %d\n", rd, rs1, immi);
    gpregisters[rd-1] = rs1 + immi;
}

void sub(INSTRUCTION instruction) {
    // Type: R
    // opcode: 0110011
    // func3: 000
    // func7: 0100000

}

void lui(INSTRUCTION instruction) {
    // Type: U
    // opcode: 0110111

}

void xor(INSTRUCTION instruction) {
    // Type: R
    // opcode: 0110011
    // func3: 100
    // fun7: 0000000

}

void xori(INSTRUCTION instruction) {
    // Type: I
    // opcode: 0010011
    // func3: 100

}

void or(INSTRUCTION instruction) {
    // Type: R
    // opcode: 0011011
    // func3: 110
    // func7: 0000000

}

void ori(INSTRUCTION instruction) {
    // Type: I
    // opcode: 0010011
    // func3: 110

}

void and(INSTRUCTION instruction) {
    // Type: R
    // opcode: 0110011
    // func3: 111
    // func7: 0000000

}

void andi(INSTRUCTION instruction) {
    // Type: I
    // opcode: 0010011
    // func3: 111

}

void sll(INSTRUCTION instruction) {
    // Type: R
    // func3: 001
    // fun7: 0000000

}

void srl(INSTRUCTION instruction) {
    // Type: R
    // opcode: 0110011
    // func3: 101
    // func7: 0000000

}

void sra(INSTRUCTION instruction) {
    // Type: R
    // opcode: 0110011
    // func3: 101
    // func7: 0100000

}

void lb(INSTRUCTION instruction) {
    // Type: I
    // opcode: 0110011
    // func3: 101

}

void lh(INSTRUCTION instruction) {
    // Type: I
    // opcode: 0000011
    // func3: 001

}

void lw(INSTRUCTION instruction) {
    // Type: I
    // opcode: 0000011
    // func3: 010

}

void lbu(INSTRUCTION instruction) {
    // Type: I
    // opcode: 0000011
    // func3: 100

}

void lhu(INSTRUCTION instruction) {
    // Type: I
    // opcode: 0000011
    // func3: 101

}

void sb(INSTRUCTION instruction) {
    // Type: S
    // opcode: 0100011
    // func3: 000

}

void sh(INSTRUCTION instruction) {
    // Type: S
    // opcode: 0100011
    // func3: 001

}

void sw(INSTRUCTION instruction) {
    // Type: S
    // opcode: 0100011
    // func3: 010

}

void inst_r(INSTRUCTION instruction) {

}

void process_instruction(INSTRUCTION instruction) {
    unsigned int opcode = mask(instruction, 0, 6);
    unsigned int func3 = mask(instruction, 12, 14);
    unsigned int func7 = mask(instruction, 25, 31);


    // printf("instruction: ");
    // print_binary(instruction);
    // printf("\n");
    // printf("opcode: ");
    // print_binary(opcode);
    // printf("\n");

    switch (opcode) {
        case R:
            if (func3 == 0 && func7 == 0) {
                add(instruction);
            }
            break;
        case I:
            if (!func3) {
                addi(instruction);
            }
            break;
        case S:
            break;
        case SB:
            break;
        case U:
            break;
        case UJ:
            break;
    }
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

    // printf("%d\n", mask(instructions[0], 0, 6));

    // Print out all instructions (for debugging)
    // for (int i = 0; i < 1024; i++) {
    //     printf("%08x\n", instructions[i]);
    // }

    process_instruction(instructions[0]);

    for (int i = 0; i < 31; i++) {
        printf("register %02d: %d\n", i+1, gpregisters[i]);
    }

    return 0;
}
