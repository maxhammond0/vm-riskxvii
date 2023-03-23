// name: Max Hammond
// unikey: mham5835
// SID: 520477289

int debug = 1;

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
#define DATA_MEM_SIZE 2048

// program counter
int pc = 0;

// 0 register and 31 general purpose registers
const int r0 = 0;
int reg[32] = {0};

// helper function to check bit masking
void print_binary(unsigned int number) {
    if (number >> 1) {
        print_binary(number >> 1);
    }
    putc((number & 1) ? '1' : '0', stdout);
}

void register_dump() {
    // printf("PC = %08d\n", pc*4);
    for (int i = 0; i < 32; i++) {
        printf("R[%d] =  %08d\n", i, reg[i]);
    }
}

unsigned int mask(INSTRUCTION n, int i, int j) {
    // Return the ith to the jth bits of an INSTRUCTION
    int p = j - i + 1;
    i++;
    return (((1 << p) - 1) & (n >> (i - 1)));
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

void r(INSTRUCTION instruction) {

    unsigned int rd = mask(instruction, 7, 11);
    unsigned int func3 = mask(instruction, 12, 14);
    unsigned int rs1 = mask(instruction, 15, 19);
    unsigned int rs2 = mask(instruction, 20, 24);
    unsigned int func7 = mask(instruction, 25, 31);


    if (func3 == 0b000 && func7 == 0b0000000) {  // add
        if (debug) printf("add: r[%d] = r%d(%d) + r%d(%d)", rd, rs1, reg[rs1], rs2, reg[rs2]);
        reg[rd] = reg[rs1] + reg[rs2];
    }
    else if (func3 == 0b000 && func7 == 0b0100000) {  // sub
        if (debug) printf("sub: r[%d] = r%d(%d) - r%d(%d)", rd, rs1, reg[rs1], rs2, reg[rs2]);
        reg[rd] = reg[rs1] - reg[rs2];
    }
    else if (func3 == 0b100 && func7 == 0b0000000) {  // xor
        if (debug) printf("xor: r[%d] = r%d(%d) ^ r%d(%d)", rd, rs1, reg[rs1], rs2, reg[rs2]);
        reg[rd] = reg[rs1] ^ reg[rs2];
    }
    else if (func3 == 0b110 && func7 == 0b0000000) {  // or
        if (debug) printf("or: r[%d] = r%d(%d) | r%d(%d)", rd, rs1, reg[rs1], rs2, reg[rs2]);
        reg[rd] = reg[rs1] | reg[rs2];
    }
    else if (func3 == 0b111 && func7 == 0b0000000) {  // and
        if (debug) printf("and: r[%d] = r%d(%d) & r%d(%d)", rd, rs1, reg[rs1], rs2, reg[rs2]);
        reg[rd] = reg[rs1] & reg[rs2];
    }
    else if (func3 == 0b001 && func7 == 0b0000000) {  // sll
        if (debug) printf("sll: r[%d] = r%d(%d) << r%d(%d)", rd, rs1, reg[rs1], rs2, reg[rs2]);
        reg[rd] = reg[rs1] << reg[rs2];
    }
    else if (func3 == 0b101 && func7 == 0b0000000) {  // srl
        if (debug) printf("srl: r[%d] = r%d(%d) >> r%d(%d)", rd, rs1, reg[rs1], rs2, reg[rs2]);
        reg[rd] = reg[rs1] >> reg[rs2];
    }
    else if (func3 == 0b101 && func7 == 0b0100000) {  // sra
        // TODO read spec and fix
        if (debug) printf("sra: r[%d] = r%d(%d) >> r%d(%d)", rd, rs1, reg[rs1], rs2, reg[rs2]);
        reg[rd] = reg[rs1] >> reg[rs2];
    }
    else if (func3 == 0b010 && func7 == 0b0000000) {  // slt
        if (debug) printf("slt: r[%d] = r%d(%d) < r%d(%d) ? 1 : 0", rd, rs1, reg[rs1], rs2, reg[rs2]);
        reg[rd] = (reg[rs1] < reg[rs2]) ? 1 : 0;
    }
    else if (func3 == 0b011 && func7 == 0b0000000) {  // sltu
        if (debug) printf("sltu: r[%d] = r%d(%d) < r%d(%d) ? 1 : 0", rd, rs1, reg[rs1], rs2, reg[rs2]);
        reg[rd] = ((uint32_t)reg[rs1] <
            (uint32_t)reg[rs2]) ?
            1 :
            0;
    }
}

void i(INSTRUCTION instruction, uint8_t data_mem[DATA_MEM_SIZE]) {

    uint8_t opcode = mask(instruction, 0, 6);
    unsigned int rd = mask(instruction, 7, 11);
    unsigned int func3 = mask(instruction, 12, 14);
    unsigned int rs1 = mask(instruction, 15, 19);
    uint32_t imm = mask(instruction, 20, 31);

    uint32_t unsigned_imm = imm;

    // Sign the immediate
    if ((imm >> 11) & 1) {
        imm = imm | 4294965248;  //
    }

    if (opcode == 0b0010011) {
        if (func3 == 0b000) {  // addi
            if (debug) printf("addi: r[%d] = r%d(%d) + (%d)", rd, rs1, reg[rs1], imm);
            reg[rd] = reg[rs1] + imm;
        } else if (func3 == 0b100) {  // xori
            if (debug) printf("xori: r[%d] = r%d(%d) ^ (%d)", rd, rs1, reg[rs1], imm);
            reg[rd] = reg[rs1] ^ imm;
        } else if (func3 == 0b110) {  // ori
            if (debug) printf("andi: r[%d] = r%d(%d) | (%d)", rd, rs1, reg[rs1], imm);
            reg[rd] = reg[rs1] | imm;
        } else if (func3 == 0b111) {  // andi
            if (debug) printf("slti: r[%d] = r%d(%d) & (%d)", rd, rs1, reg[rs1], imm);
            reg[rd] = reg[rs1] & imm;
        } else if (func3 == 0b010) {  // slti
            if (debug) printf("slti: r[%d] = r%d(%d) < (%d) ? 1 : 0", rd, rs1, reg[rs1], imm);
            reg[rd] = (reg[rs1] < imm) ? 1 : 0;
        } else if (func3 == 0b011) {  // sltiu
            if (debug) printf("sltiu: r[%d] = r%d(%d) < (%d) ? 1 : 0", (uint32_t)rd, rs1, reg[rs1], unsigned_imm);
            reg[rd] = ((uint32_t)reg[rs1] < unsigned_imm) ? 1 : 0;
        }
    } else if (opcode == 0b1100111) {
        if (func3 == 0b000) {  // jalr
            if (debug) printf("jalr: r[%d] = PC(%d)+4; PC=(r%d(%d)+%d)", rd, pc*4, rs1, reg[rs1], imm);
            reg[rd] = pc*4 + 4;
            pc = ((reg[rs1] + imm)/4)-1;
        }
    } else if (opcode == 0b0000011) {  // memory loading
        int addy = (reg[rs1] + imm);

        // Load virtual routines
        int read_c = 2066;
        int read_i = 2070;

        if (addy == read_c || addy == read_i) {
            uint32_t input;
            if (debug) {
                if (addy == read_c) printf("Enter character: ");
                else printf("Enter integer: ");
            }
            scanf("%d", &input);
            if (addy == read_c) {
                input = (char)input;
            } else {
                input = (int)input;
            }
            reg[rd] = input;
        } else {
            // addy = addy / 4;
            // printf("addy: %d", addy);
            // TODO unsure if sexted properly
            if (func3 == 0b000) {  // lb
                if (debug) printf("lb: r[%d] = %d & 0xFFFFFF00 = %d", rd, data_mem[addy], reg[rd]);
                uint32_t load = data_mem[addy] & 0x000000FF;
                reg[rd] = load & 0xFFFFFF00;
            } else if (func3 == 0b001) {  // lh
                if (debug) printf("lh: r[%d] = %d & 0xFFFF0000 = %d", rd, data_mem[addy], reg[rd]);
                uint32_t load = data_mem[addy] & 0x0000FFFF;
                reg[rd] = load & 0xFFFF0000;
            } else if (func3 == 0b010) {  // lw
                if (debug) printf("lw: r[%d] = %d", rd, data_mem[addy]);
                reg[rd] = data_mem[addy];
            } else if (func3 == 0b100) {  // lbu
                if (debug) printf("lbu: r[%d] = %d & 0x000000FF = %d", rd, data_mem[addy], reg[rd] & 0x000000FF);
                reg[rd] = data_mem[addy] & 0x000000FF;
            } else if (func3 == 0b101) {  // lhu
                if (debug) printf("lw: r[%d] = %d & 0x0000FFFF = %d", rd, data_mem[addy], reg[rd] & 0x000000FF);
                reg[rd] = data_mem[addy] & 0x0000FFFF;
            }
        }
    }
}

void s(INSTRUCTION instruction, uint8_t data_mem[DATA_MEM_SIZE]) {

    uint32_t func3 = mask(instruction, 12, 14);
    uint32_t rs1 = mask(instruction, 15, 19);
    uint32_t rs2 = mask(instruction, 20, 24);

    int imm0to4 = mask(instruction, 7, 11);
    int imm5to11 = mask(instruction, 25, 31);

    // Store virtual routines
    int write_c = 0x800;
    int write_i = 0x804;
    int write_ui = 0x808;
    int halt = 0x80c;
    int dump_pc = 0x820;
    int dump_gpr = 0x824;
    // int heap_banks = 2088;

    int imm = (imm5to11 << 5) | imm0to4;
    // sign the immediate
    if ((imm >> 11) & 1) {
        imm = imm | 4294965248;
    }

    int addy = (reg[rs1] + imm);

    if (addy == halt) {
        printf("CPU Halt Requested\n");
        if (debug) register_dump();
        exit(0);
    }

    if (addy == write_c) {
        uint8_t b = mask(reg[rs2], 0, 7);
        if (debug) printf("write character in r%d(%d): ", rs2, reg[rs2]);
        printf("%d", b);
        return;
    } else if (addy == write_i) {
        int32_t b = reg[rs2];
        if (debug) printf("write character in r%d(%d): ", rs2, reg[rs2]);
        printf("%d", b);
        return;
    } else if (addy == write_ui) {
        uint32_t b = reg[rs2];
        if (debug) printf("write character in r%d(%d): ", rs2, reg[rs2]);
        printf("%d", b);
        return;
    } else if (addy == dump_pc) {
        if (debug) printf("dump program counter: ");
        printf("%x", pc*4);
    } else if (addy == dump_gpr) {
        if (debug) printf("dump registers: \n");
        register_dump();
    } else {

        // addy = addy / 4;

        if (addy < 0 || addy > DATA_MEM_SIZE) {
            printf("\naddy: %d\n", addy);
            printf("address out of bounds\n!\n");
            register_dump();
            exit(4);
        }

        if (func3 == 0b000) {  // sb
            if (debug) printf("sb: data_mem[%d] = r%d(%d)", addy, rs2, reg[rs2]);
            uint8_t low8bits = mask(reg[rs2], 0, 7);
            data_mem[addy] = low8bits;
        } else if (func3 == 0b001) {  // sh
            if (debug) printf("sh: data_mem[%d] = r%d(%d)", addy, rs2, reg[rs2]);
            uint8_t low8bits = mask(reg[rs2], 0, 7);
            uint8_t low16bits = mask(reg[rs2], 8, 15);
            data_mem[addy+0] = low8bits;
            data_mem[addy+1] = low16bits;
        } else if (func3 == 0b010) {  // sw
            if (debug) printf("sw: data_mem[%d] = r%d(%d)", addy, rs2, reg[rs2]);
            uint8_t low8bits = mask(reg[rs2], 0, 7);
            uint8_t low16bits = mask(reg[rs2], 8, 15);
            uint8_t low24bits = mask(reg[rs2], 16, 23);
            uint8_t low32bits = mask(reg[rs2], 24, 31);
            data_mem[addy+0] = low8bits;
            data_mem[addy+1] = low16bits;
            data_mem[addy+2] = low24bits;
            data_mem[addy+3] = low32bits;
        }
    }
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


    if (func3 == 0b000) {  // beq
        if (debug) printf("if (r%d(%d) == r%d(%d)) beq: PC = PC(%d) + %d", rs1, reg[rs1], rs2, reg[rs2], pc*4, imm << 1);
        if (reg[rs1] == reg[rs2]) {
            pc = (pc*4 + (imm << 1))/4-1;
        }
    } else if (func3 == 1) {  // bne
        if (debug) printf("if (r%d(%d) != r%d(%d)) bne: PC = PC(%d) + %d", rs1, reg[rs1], rs2, reg[rs2], pc*4, imm << 1);
        if (reg[rs1] != reg[rs2]) {
            pc = (pc*4 + (imm << 1))/4-1;
        }
    } else if (func3 == 4) {  // blt
        if (debug) printf("if (r%d(%d) < r%d(%d)) blt: PC = PC(%d) + %d", rs1, reg[rs1], rs2, reg[rs2], pc*4, imm << 1);
        if (reg[rs1] < reg[rs2]) {
            pc = (pc*4 + (imm * 2))/4-1;
        }
    } else if (func3 == 6) {  // bltu
        if (debug) printf("if (r%d(%d) < r%d(%d)) bltu: PC = PC(%d) + %d", rs1, (uint32_t)reg[rs1], rs2, (uint32_t)reg[rs2], pc*4, imm << 1);
        if ((uint32_t)reg[rs1] < (uint32_t)reg[rs2]) {
            pc = (pc*4 + (imm * 2))/4-1;
        }
    } else if (func3 == 5) {  // bge
        if (debug) printf("if (r%d(%d) > r%d(%d)) bltu: PC = PC(%d) + %d", rs1, reg[rs1], rs2, reg[rs2], pc*4, imm << 1);
        if (reg[rs1] > reg[rs2]) {
            pc = (pc*4 + (imm * 2))/4-1;
            if (debug) printf("bge: PC = PC(%d) + %d", pc*4, imm << 1);
        }
    } else if (func3 == 7) {  // bgeu
        if (debug) printf("if (r%d(%d) > r%d(%d)) bltu: PC = PC(%d) + %d", rs1, (uint32_t)reg[rs1], rs2, (uint32_t)reg[rs2], pc*4, imm << 1);
        if ((uint32_t)reg[rs1] > (uint32_t)reg[rs2]) {
            pc = (pc*4 + (imm * 2))/4-1;
        }
    } else {
        // TODO error message
    }
}

void u(INSTRUCTION instruction) {
    // lui
    unsigned int rd = mask(instruction, 7, 11);
    uint32_t imm = mask(instruction, 12, 31);

    // sign the immediate
    if ((imm >> 19) & 1) {
        imm = imm | 4294965248;
    }
    imm = (imm << 12);

    reg[rd] = imm;
    if (debug) printf("lui: r[%d] = %d", rd, imm);
}

void uj(INSTRUCTION instruction) {
    // jal
    unsigned int rd = mask(instruction, 7, 11);

    // bit fuckery
    uint32_t imm20 = mask(instruction, 31, 31);
    uint32_t imm10to1 = mask(instruction, 21, 30);
    uint32_t imm11 = mask(instruction, 20, 20);
    uint32_t imm19to12 = mask(instruction, 12, 19);

    // bit shift and logical 'or'ing all them together
    uint32_t imm = (imm20 << 19) |
        (imm19to12 << 11) |
        (imm11 << 10) |
        imm10to1;

    // sign the immediate
    if ((imm >> 19) & 1) {
        imm = imm | 4294965248;
    }

    if (debug) printf("jal: r[%d] = %d + 4; pc = %d + %d", rd, pc*4, pc*4, imm << 1);
    reg[rd] = pc*4 + 4;
    pc = ((pc*4 + (imm<<1))/4)-1;
}

void process_instruction(INSTRUCTION instruction,
                         uint8_t data_mem[DATA_MEM_SIZE]) {

    unsigned int opcode = mask(instruction, 0, 6);

    switch (opcode) {
        case R:
            r(instruction);
            break;
        case I:
        case 3:  // memory load
        case 103:  // jalr
            i(instruction, data_mem);
            break;
        case S:
            s(instruction, data_mem);
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
        case 0:
            break;
        default:
            printf("opcode not found, ");
            printf("opcode was: ");
            print_binary(opcode);
    }
    reg[0] = 0;
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

    // Run program
    for ( ; pc < INST_MEM_SIZE; pc++) {
        if (debug) printf("pc: %04d, ", pc*4);
        process_instruction(instructions[pc], data_mem);
        if (debug) printf("\n");
    }

    return 0;
}
