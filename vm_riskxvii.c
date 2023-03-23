// name: Max Hammond
// unikey: mham5835
// SID: 520477289

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <limits.h>

typedef uint32_t INSTRUCTION;

#define R 51
#define I 19
#define S 35
#define SB 99
#define U 55
#define UJ 111

#define INST_MEM_SIZE 256
#define DATA_MEM_SIZE 1024

// program counter
int pc = 0;

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
    printf("PC = %08d\n", pc*4);
    for (int i = 0; i < 32; i++) {
        printf("R[%d] =  %08d\n", i, gpregisters[i]);
    }
}

unsigned int mask(INSTRUCTION n, int i, int j) {
    // Return the ith to the jth bits of an INSTRUCTION
    int p = j - i + 1;
    i++;
    return (((1 << p) - 1) & (n >> (i - 1)));
}


void process_r(INSTRUCTION instruction) {
    // unsigned int rd = mask(instruction, 7, 11);
    // unsigned int func3 = mask(instruction, 12, 14);
    // unsigned int rs1 = mask(instruction, 15, 19);
    // unsigned int rs2 = mask(instruction, 20, 24);
    // unsigned int func7 = mask(instruction, 25, 31);
}

void process_i(INSTRUCTION instruction) {

}

void process_s(INSTRUCTION instruction) {

}

void process_sb(INSTRUCTION instruction) {

}

void process_u(INSTRUCTION instruction) {

}

void process_uj(INSTRUCTION instruction) {

}

void get_instructions(char *filepath, INSTRUCTION *instructions) {
    // Reads file and loads instructions into the instructions array
    int fd;
    uint8_t buffer[4] = {0};

    // If file can't be read
    if ((fd = open(filepath, O_RDONLY)) < 0) {
        printf("Could not open file\n");
        printf("Exiting...\n");
        exit(2);
    }

    int i = 0;
    // Read first 256 instructions
    while (i < 256) {
        read(fd, &buffer, 4);
        INSTRUCTION op = 0u;
        op |= buffer[3] << 24;  // 0xAA000000
        op |= buffer[2] << 16;  // 0xaaBB0000
        op |= buffer[1] << 8;   // 0xaabbCC00
        op |= buffer[0];        // 0xaabbccDD
        instructions[i] = op;
        i++;
    }
}

void get_data(char *filepath, INSTRUCTION *data_mem) {
    // TODO
}

void interpret_instructions(INSTRUCTION instruction) {
    uint8_t opcode = mask(instruction, 0, 7);

    switch (opcode) {
        case R:
            break;
        case I:
            break;
        case S:
            break;
        case 3:
            break;
        case SB:
            break;
        case U:
            break;
        case UJ:
            break;
        case 103:
            break;
        case 0:
            break;
        default:
            printf("opcode not found, ");
            printf("opcode was: ");
            print_binary(opcode);
    }
}

void r(INSTRUCTION instruction) {

    printf("Type: R, ");

    unsigned int rd = mask(instruction, 7, 11);
    unsigned int func3 = mask(instruction, 12, 14);
    unsigned int rs1 = mask(instruction, 15, 19);
    unsigned int rs2 = mask(instruction, 20, 24);
    unsigned int func7 = mask(instruction, 25, 31);


    if (func3 == 0 && func7 == 0) {  // add
        gpregisters[rd] = gpregisters[rs1] + gpregisters[rs2];
    } else if (func3 == 0 && func7 == 32) {  // sub
        gpregisters[rd] = gpregisters[rs1] - gpregisters[rs2];
    } else if (func3 == 4 && func7 == 0) {  // xor
        gpregisters[rd] = gpregisters[rs1] ^ gpregisters[rs2];
    } else if (func3 == 6 && func7 == 0) {  // or
        gpregisters[rd] = gpregisters[rs1] | gpregisters[rs2];
    } else if (func3 == 7 && func7 == 0) {  // and
        gpregisters[rd] = gpregisters[rs1] & gpregisters[rs2];
    } else if (func3 == 1 && func7 == 0) {  // sll
        gpregisters[rd] = gpregisters[rs1] << gpregisters[rs2];
    } else if (func3 == 5 && func7 == 0) {  // srl
        gpregisters[rd] = gpregisters[rs1] >> gpregisters[rs2];
    } else if (func3 == 5 && func7 == 32) {  // sra
        gpregisters[rd] = gpregisters[rs1] >> gpregisters[rs2];
    } else if (func3 == 2 && func7 == 0) {  // slt
        gpregisters[rd] = (gpregisters[rs1] < gpregisters[rs2]) ? 1 : 0;
    } else if (func3 == 3 && func7 == 0) {  // sltu
        gpregisters[rd] = ((uint32_t)gpregisters[rs1] <
            (uint32_t)gpregisters[rs2]) ?
            1 :
            0;
    }

    // debugging

    gpregisters[0] = 0;
}

void i(INSTRUCTION instruction) {


    unsigned int rd = mask(instruction, 7, 11);
    unsigned int func3 = mask(instruction, 12, 14);
    unsigned int rs1 = mask(instruction, 15, 19);
    uint32_t imm = mask(instruction, 20, 31);

    uint32_t unsigned_imm = imm;

    // Sign the immediate
    if ((imm >> 11) & 1) {
        imm = imm | 4294965248;  //
    }

    if (func3 == 0) {  // addi
        gpregisters[rd] = gpregisters[rs1] + imm;
    } else if (func3 == 4) {  // xori
        gpregisters[rd] = gpregisters[rs1] ^ imm;
    } else if (func3 == 6) {  // ori
        gpregisters[rd] = gpregisters[rs1] | imm;
    } else if (func3 == 7) {  // andi
        gpregisters[rd] = gpregisters[rs1] & imm;
    } else if (func3 == 2) {  // slti
        gpregisters[rd] = (gpregisters[rs1] < imm) ? 1 : 0;
    } else if (func3 == 3) {  // sltiu
        gpregisters[rd] = ((uint32_t)gpregisters[rs1] < unsigned_imm) ? 1 : 0;
    }

    // debugging

    gpregisters[0] = 0;
}

void s(INSTRUCTION instruction, uint8_t data_mem[DATA_MEM_SIZE]) {


    unsigned int func3 = mask(instruction, 12, 14);
    unsigned int rs1 = mask(instruction, 15, 19);
    unsigned int rs2 = mask(instruction, 20, 24);

    uint32_t imm1to5 = mask(instruction, 7, 11);
    int imm6to12 = mask(instruction, 25, 31);

    // Store virtual routines
    int write_c = 2048;
    int write_i = 2052;
    int write_ui = 2056;
    int halt = 2060;
    // int heap_banks = 2088;
    // int dump_pc = 2080;
    // int dump_gpr = 2084;

    int32_t imm = (imm6to12 << 5) | imm1to5;


    // sign the immediate
    if ((imm >> 11) & 1) {
        imm = imm | 4294965248;
    }

    uint32_t addy = (gpregisters[rs1] + imm);

    if (addy == halt) {
        printf("CPU halt requested\n");
        // register_dump();
        exit(0);
    }



    if (addy == write_c) {
        uint8_t b = mask(gpregisters[rs2], 0, 7);
        printf("%c", b);
        return;
    } else if (addy == write_i) {
        int32_t b = gpregisters[rs2];
        printf("%d", b);
        return;
    } else if (addy == write_ui) {
        uint32_t b = gpregisters[rs2];
        printf("%d", b);
        return;
    }

    addy = addy /4;

    if (addy < 0 || addy > DATA_MEM_SIZE) {
        printf("\n%d\n", addy);
        printf("address out of bounds\n!\n");
        return;
    }

    if (func3 == 0) {  // sb
        uint8_t low8bits = mask(gpregisters[rs2], 0, 7);
        data_mem[addy] = low8bits;
    } else if (func3 == 1) {  // sh
        uint8_t low8bits = mask(gpregisters[rs2], 0, 7);
        uint8_t low16bits = mask(gpregisters[rs2], 8, 15);
        data_mem[addy+0] = low8bits;
        data_mem[addy+1] = low16bits;
    } else if (func3 == 2) {  // sw
        uint8_t low8bits = mask(gpregisters[rs2], 0, 7);
        uint8_t low16bits = mask(gpregisters[rs2], 8, 15);
        uint8_t low24bits = mask(gpregisters[rs2], 16, 23);
        uint8_t low32bits = mask(gpregisters[rs2], 24, 31);
        data_mem[addy+0] = low8bits;
        data_mem[addy+1] = low16bits;
        data_mem[addy+2] = low24bits;
        data_mem[addy+3] = low32bits;
    } else {
    }
    gpregisters[0] = 0;
}

void memory_load(INSTRUCTION instruction,
                 uint8_t data_mem[DATA_MEM_SIZE]) {


    unsigned int rd = mask(instruction, 7, 11);
    unsigned int func3 = mask(instruction, 12, 14);
    unsigned int rs1 = mask(instruction, 15, 19);
    uint32_t imm = mask(instruction, 20, 31);

    // sign the immediate
    if ((imm >> 11) & 1) {
        imm = imm | 4294965248;
    }

    int addy = (gpregisters[rs1] + imm);

    // Load virtual routines
    int read_c = 2066;
    int read_i = 2070;

    if (addy == read_c || addy == read_i) {
        // TODO read character
        uint32_t input;
        scanf("%d", &input);
        gpregisters[rd] = input;
    } else {
        // NOT GETTING USER INPUT
        addy = addy / 4;

        if (addy < 0 || addy > DATA_MEM_SIZE) {
            printf("\n%d\n", addy);
            printf("address out of bounds\n!\n!\n!\n!\n!\n");
            // printf("exiting");
            // exit(3);
        }

        if (func3 == 0) {  // lb
        } else if (func3 == 1) {  // lh
        } else if (func3 == 2) {  // lw
            uint8_t byte1 = data_mem[addy+3];
            uint8_t byte2 = data_mem[addy+2];
            uint8_t byte3 = data_mem[addy+1];
            uint8_t byte4 = data_mem[addy+0];
            gpregisters[rd] = (byte1 << 24) |
                (byte2 << 16) |
                (byte3 << 8) |
                byte4;
        } else if (func3 == 4) {  // lbu
            gpregisters[rd] = data_mem[addy];
        } else if (func3 == 5) {  // lhu
            uint8_t byte1 = data_mem[addy+1];
            uint8_t byte2 = data_mem[addy+0];
            gpregisters[rd] = (byte1 << 8) |
                byte2;
        } else {
            // TODO error message
        }
    }

    // debugging

    gpregisters[0] = 0;
}

void sb(INSTRUCTION instruction) {


    unsigned int func3 = mask(instruction, 12, 14);
    unsigned int rs1 = mask(instruction, 15, 19);
    unsigned int rs2 = mask(instruction, 20, 24);

    uint32_t imm11 = mask(instruction, 7, 7);
    uint32_t imm1to4 = mask(instruction, 8, 11);
    uint32_t imm12 = mask(instruction, 31, 31);
    uint32_t imm5to10 = mask(instruction, 25, 30);

    int32_t imm = (imm12 << 11) |
        (imm11 << 10) |
        (imm5to10 << 4) |
        (imm1to4);

    // sign the immediate
    if ((imm >> 11) & 1) {
        imm = imm | 4294965248;
    }


    if (func3 == 0) {  // beq
        if (gpregisters[rs1] == gpregisters[rs2]) {
            pc = (pc*4 + (imm * 2))/4-1;
        }
    } else if (func3 == 1) {  // bne
        if (gpregisters[rs1] != gpregisters[rs2]) {
            pc = (pc*4 + (imm * 2))/4-1;
        }
    } else if (func3 == 4) {  // blt
        if (gpregisters[rs1] < gpregisters[rs2]) {
            pc = (pc*4 + (imm * 2))/4-1;
        }
    } else if (func3 == 6) {  // bltu
        if ((uint32_t)gpregisters[rs1] < (uint32_t)gpregisters[rs2]) {
            pc = (pc*4 + (imm * 2))/4-1;
        }
    } else if (func3 == 5) {  // bge
        if (gpregisters[rs1] > gpregisters[rs2]) {
            pc = (pc*4 + (imm * 2))/4-1;
        }
    } else if (func3 == 7) {  // bgeu
        if ((uint32_t)gpregisters[rs1] > (uint32_t)gpregisters[rs2]) {
            pc = (pc*4 + (imm * 2))/4-1;
        }
    } else {
        // TODO error message
    }
    // debugging

    gpregisters[0] = 0;
}

void u(INSTRUCTION instruction) {
    // lui

    unsigned int rd = mask(instruction, 7, 11);
    uint32_t imm = mask(instruction, 12, 31);
    imm = (imm << 12);

    // sign the immediate
    if ((imm >> 19) & 1) {
        imm = imm | 4294965248;
    }

    // debugging

    gpregisters[rd] = imm;
    gpregisters[0] = 0;
}

void uj(INSTRUCTION instruction) {
    // jal


    unsigned int rd = mask(instruction, 7, 11);

    // bit fuckery
    uint32_t imm20 = mask(instruction, 31, 31);
    uint32_t imm10to1 = mask(instruction, 21, 30);
    uint32_t imm11 = mask(instruction, 20, 20);
    uint32_t imm19to12 = mask(instruction, 12, 19);

    // bit shift and logical oring all them together
    uint32_t imm = (imm20 << 19) |
        (imm19to12 << 11) |
        (imm11 << 10) |
        imm10to1;

    // sign the imm
    if ((imm >> 19) & 1) {
        imm = imm | 4294965248;
    }

    gpregisters[rd] = pc*4 + 4;

    pc = ((pc*4 + (imm*2))/4)-1;


    gpregisters[0] = 0;
}

void jalr(INSTRUCTION instruction) {


    unsigned int rd = mask(instruction, 7, 11);
    // unsigned int func3 = mask(instruction, 12, 14);
    unsigned int rs1 = mask(instruction, 15, 19);
    int imm = mask(instruction, 20, 31);

    gpregisters[rd] = pc*4 + 4;
    pc = ((gpregisters[rs1] + imm)/4)-1;


    gpregisters[0] = 0;
}

void process_instruction(INSTRUCTION instruction,
                         uint8_t data_mem[DATA_MEM_SIZE]) {
    unsigned int opcode = mask(instruction, 0, 6);

    switch (opcode) {
        case R:
            r(instruction);
            break;
        case I:
            i(instruction);
            break;
        case S:
            s(instruction, data_mem);
            break;
        case 3:
            memory_load(instruction, data_mem);
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

    INSTRUCTION instructions[INST_MEM_SIZE] = { 0 };
    uint8_t data_mem[DATA_MEM_SIZE];

    get_instructions(argv[1], instructions);

    // print instructions for debugging
    // for (int i = 0; i < INST_MEM_SIZE; i++) {
    //     printf("%04d: %08x\n", i*4, instructions[i]);
    // }

    // Run program
    for ( ; pc < INST_MEM_SIZE; pc++) {
        // printf("pc: %04d, ", pc*4);
        process_instruction(instructions[pc], data_mem);
        // printf("\n");
    }

    return 0;
}
