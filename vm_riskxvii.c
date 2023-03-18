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

// program counter
int pc = 0;

typedef uint32_t INSTRUCTION;

// 0 register and 31 general purpose registers
const int r0 = 0;
int gpregisters[32] = {0};

// helper function to check bit masking
void print_binary(unsigned int number) {
    if (number >> 1) {
        print_binary(number >> 1);
    }
    putc((number & 1) ? '1' : '0', stdout);
}

void register_dump() {
    for (int i = 0; i < 32; i++) {
        printf("R%02d: %d\n", i, gpregisters[i]);
    }
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

    int i = 0;
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
            op |= (unsigned int)*byte1 << 24;  // 0xAA000000
            op |= (unsigned int)*byte2 << 16;  // 0xaaBB0000
            op |= (unsigned int)*byte3 << 8;   // 0xaabbCC00
            op |= (unsigned int)*byte4;        // 0xaabbccDD
            instructions[i] = op;
            i++;
        }
    }
    gpregisters[0] = 0;
}

unsigned int mask(INSTRUCTION n, int i, int j) {
    // Return the ith to the jth bits of an INSTRUCTION
    int p = j - i + 1;
    i++;
    return (((1 << p) - 1) & (n >> (i - 1)));
}

void r(INSTRUCTION instruction) {
    unsigned int rd = mask(instruction, 7, 11);
    unsigned int func3 = mask(instruction, 12, 14);
    unsigned int rs1 = mask(instruction, 15, 19);
    unsigned int rs2 = mask(instruction, 20, 24);
    unsigned int func7 = mask(instruction, 25, 31);


    if (func3 == 0 && func7 == 0) {  // add
        printf("add ");
        gpregisters[rd] = gpregisters[rs1] + gpregisters[rs2];
    } else if (func3 == 0 && func7 == 32) {  // sub
        printf("sub ");
        gpregisters[rd] = gpregisters[rs1] - gpregisters[rs2];
    } else if (func3 == 4 && func7 == 0) {  // xor
        printf("xor ");
        gpregisters[rd] = gpregisters[rs1] ^ gpregisters[rs2];
    } else if (func3 == 6 && func7 == 0) {  // or
        printf("or ");
        gpregisters[rd] = gpregisters[rs1] | gpregisters[rs2];
    } else if (func3 == 7 && func7 == 0) {  // and
        printf("and ");
        gpregisters[rd] = gpregisters[rs1] & gpregisters[rs2];
    } else if (func3 == 1 && func7 == 0) {  // sll
        printf("sll ");
        gpregisters[rd] = gpregisters[rs1] << gpregisters[rs2];
    } else if (func3 == 5 && func7 == 0) {  // srl
        printf("srl ");
        gpregisters[rd] = gpregisters[rs1] >> gpregisters[rs2];
    } else if (func3 == 5 && func7 == 32) {  // sra
        printf("sra ");
        gpregisters[rd] = gpregisters[rs1] >> gpregisters[rs2];
    } else if (func3 == 2 && func7 == 0) {  // slt
        printf("slt ");
        gpregisters[rd] = (gpregisters[rs1] < gpregisters[rs2]) ? 1 : 0;
    } else if (func3 == 3 && func7 == 0) {  // sltu
        printf("sltu ");
        // TODO treat numbers as unsigned
        gpregisters[rd] = (gpregisters[rs1] < gpregisters[rs2]) ? 1 : 0;
    }
    printf("rd: %d, rs1: %d, rs2: %d", rd, rs1, rs2);
    gpregisters[0] = 0;
}

void i(INSTRUCTION instruction) {
    unsigned int rd = mask(instruction, 7, 11);
    unsigned int func3 = mask(instruction, 12, 14);
    unsigned int rs1 = mask(instruction, 15, 19);
    unsigned int imm = mask(instruction, 20, 31);


    if (func3 == 0) {  // addi
        printf("addi ");
        gpregisters[rd] = gpregisters[rs1] + imm;
    } else if (func3 == 4) {  // xori
        printf("xori ");
        gpregisters[rd] = gpregisters[rs1] ^ imm;
    } else if (func3 == 6) {  // ori
        printf("ori ");
        gpregisters[rd] = gpregisters[rs1] | imm;
    } else if (func3 == 7) {  // andi
        printf("andi ");
        gpregisters[rd] = gpregisters[rs1] & imm;
    } else if (func3 == 2) {  // slti
        printf("slti ");
        gpregisters[rd] = (gpregisters[rs1] < imm) ? 1 : 0;
    } else if (func3 == 3) {  // slti
        printf("sltu ");
        // TODO treat numbers as unsigned
        gpregisters[rd] = (gpregisters[rs1] < imm) ? 1 : 0;
    }
    printf("rd: %d, rs1: %d, imm: %d", rd, rs1, imm);
    gpregisters[0] = 0;
}

void s(INSTRUCTION instruction) {
    unsigned int func3 = mask(instruction, 12, 14);
    unsigned int rs1 = mask(instruction, 15, 19);
    unsigned int rs2 = mask(instruction, 20, 24);

    unsigned int imm1to5 = mask(instruction, 7, 11);
    unsigned int imm6to12 = mask(instruction, 25, 31);

    // Virtual routines
    // int write_c = 2048;
    // int write_i = 2052;
    // int write_ui = 2056;
    int halt = 2060;
    // int read_c = 2066;
    // int read_i = 2070;
    // int dump_pc = 2080;
    // int dump_gpr = 2084;
    // int heap_banks = 2088;

    unsigned int imm = (imm6to12 << 5) | imm1to5;

    unsigned int addy = (gpregisters[rs1] + imm);
    printf("addy: %d, ", addy);
    printf("imm: %d, ", imm);
    print_binary(imm);
    printf(" ");
    printf("instruction ");
    print_binary(instruction);
    printf("\n");
    register_dump();
    exit(1);

    if (addy == halt) {
        printf("CPU halt requested");
        register_dump();
        exit(2);
    }

    if (func3 == 0) { // sb
        printf("sb ");
    } else if (func3 == 1) {  // sh
        printf("sh ");
    } else if (func3 == 2) {  // sw
        printf("sw ");
    }

    printf("rs1: %d, rs2: %d, imm: %d ", rs1, rs2, imm);
    gpregisters[0] = 0;
}

void memory_load(INSTRUCTION instruction) {
    // type I
    unsigned int rd = mask(instruction, 7, 11);
    unsigned int func3 = mask(instruction, 12, 14);
    unsigned int rs1 = mask(instruction, 15, 19);
    unsigned int imm = mask(instruction, 20, 31);


    if (func3 == 0) {  // lb
        printf("lb ");
    } else if (func3 == 1) {  // lh
        printf("lh ");
    } else if (func3 == 2) {  // lw
        printf("lw ");
    } else if (func3 == 4) {  // lbu
        printf("lbu ");
    } else if (func3 == 5) {  // lhu
        printf("lhu");
    }

    printf("rd: %d, rs1: %d, imm: %d", rd, rs1, imm);
}

void sb(INSTRUCTION instruction) {
    unsigned int func3 = mask(instruction, 12, 14);
    unsigned int rs1 = mask(instruction, 15, 19);
    unsigned int rs2 = mask(instruction, 20, 24);

    unsigned int imm11 = mask(instruction, 7, 7);
    unsigned int imm1to4 = mask(instruction, 8, 11);
    unsigned int imm12 = mask(instruction, 31, 31);
    unsigned int imm5to10 = mask(instruction, 25, 30);


    unsigned int imm = (imm12 << 11) |
        (imm11 << 10) |
        (imm5to10 << 4) |
        (imm1to4);

    if (func3 == 0) {  // beq
        printf("beq ");
        if (gpregisters[rs1] == gpregisters[rs2]) {
            pc = (pc*4 + (imm * 2))/4-1;
        }
    } else if (func3 == 1) {  // bne
        printf("bne ");
        if (gpregisters[rs1] != gpregisters[rs2]) {
            pc = (pc*4 + (imm * 2))/4-1;
        }
    } else if (func3 == 4) {  // blt
        printf("blt ");
        if (gpregisters[rs1] < gpregisters[rs2]) {
            pc = (pc*4 + (imm * 2))/4-1;
        }
    } else if (func3 == 6) {  // bltu
        printf("bltu ");
        // TODO treat numbers as unsigned
        if (gpregisters[rs1] < gpregisters[rs2]) {
            pc = (pc*4 + (imm * 2))/4-1;
        }
    } else if (func3 == 5) {  // bge
        printf("bge ");
        if (gpregisters[rs1] > gpregisters[rs2]) {
            pc = (pc*4 + (imm * 2))/4-1;
        }
    } else if (func3 == 7) {  // bgeu
        printf("bgeu ");
        // TODO treat numbers as unsigned
        if (gpregisters[rs1] > gpregisters[rs2]) {
            pc = (pc*4 + (imm * 2))/4-1;
        }
    }
    gpregisters[0] = 0;
    printf("rs1: %d, rs2: %d, imm: %d", rs1, rs2, imm);
}

void u(INSTRUCTION instruction) {
    // lui
    unsigned int rd = mask(instruction, 7, 11);
    unsigned int imm = mask(instruction, 12, 31);
    imm = (imm << 12);

    printf("rd: %d, imm: %d", rd, imm);

    gpregisters[rd] = imm;
}

void uj(INSTRUCTION instruction) {
    // jal
    unsigned int rd = mask(instruction, 7, 11);

    // bit fuckery
    unsigned int imm20 = mask(instruction, 31, 31);
    unsigned int imm10to1 = mask(instruction, 21, 30);
    unsigned int imm11 = mask(instruction, 20, 20);
    unsigned int imm19to12 = mask(instruction, 12, 19);

    // bit shift and logical oring all them together
    unsigned int imm = (imm20 << 20) |
        (imm19to12 << 12) |
        (imm11 << 11) |
        imm10to1;

    printf("jal ");
    printf("rd: %d, imm: %d", rd, imm);

    gpregisters[rd] = pc*4 + 4;
    pc = (pc*4 + (imm * 2))/4-1;
}

void jalr(INSTRUCTION instruction) {
    unsigned int rd = mask(instruction, 7, 11);
    unsigned int func3 = mask(instruction, 12, 14);
    unsigned int rs1 = mask(instruction, 15, 19);
    unsigned int imm = mask(instruction, 20, 31);

    printf("jalr, rd: %d, rs1: %d, imm: %d", rd, rs1, imm);

    if (func3) {
        // TODO error, func3 != 000
    }

    gpregisters[rd] = pc*4 + 4;
    pc = ((gpregisters[rs1] + imm)/4)-1; 
    gpregisters[0] = 0;
}

void process_instruction(INSTRUCTION instruction) {
    unsigned int opcode = mask(instruction, 0, 6);

    switch (opcode) {
        case R:
            r(instruction);
            break;
        case I:
            i(instruction);
            break;
        case S:
            s(instruction);
            break;
        case 3:
            memory_load(instruction);
            break;
        case SB:
            sb(instruction);
            break;
        case U:
            u(instruction);
            break;
        case UJ:
            uj(instruction);
            break;
        case 103:
            jalr(instruction);
            break;
        case 0:
            break;
        default:
            printf("opcode not found, ");
            printf("opcode was: ");
            print_binary(opcode);
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

    // Run program
    for ( ; pc < 32; pc++) {
        printf("pc: %03d, ", pc*4);
        process_instruction(instructions[pc]);
        printf("\n");
    }

    register_dump();

    return 0;
}
