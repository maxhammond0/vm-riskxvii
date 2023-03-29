// name: Max Hammond
// unikey: mham5835
// SID: 520477289

int debug = 0;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <limits.h>

typedef uint32_t INSTRUCTION;
typedef struct node node_t;

#define R 51
#define I 19
#define S 35
#define SB 99
#define U 55
#define UJ 111

#define INST_MEM_SIZE 1024
#define DATA_MEM_SIZE 1024
#define HBANK_SIZE 64

// program counter
int pc = 0;

// 0 register and 31 general purpose registers
uint32_t reg[32] = {0};

// TODO Initialize heap bank

// helper function to check bit masking
void print_binary(unsigned int number) {
    if (number >> 1) {
        print_binary(number >> 1);
    }
    putc((number & 1) ? '1' : '0', stdout);
}

void register_dump() {
    for (int i = 0; i < 32; i++) {
        printf("R[%d] = 0x%08x;\n", i, reg[i]);
    }
}

struct node {
    uint8_t data[HBANK_SIZE];
    uint32_t location;
    node_t* next;
};

node_t* heap_init() {
    node_t* head = (node_t*)malloc(sizeof(node_t));
    head->location = 0xb700-64;

    // Initialize new data to 0;
    for (int i = 0; i < HBANK_SIZE; i++) {
        head->data[i] = 0;
    }
    head->next = NULL;

    return head;
}

int heap_add(node_t** head, int size) {

    int num_of_banks;
    if (size % 64 == 0) {
        num_of_banks = (size / 64);
    } else {
        num_of_banks = (size / 60) + 1;
    }

    node_t* cursor = *head;
    while (cursor->next) {
        cursor = cursor->next;
    }

    int location = cursor->location + 64;

    for (int i = 0; i < num_of_banks; i++) {

        int tmp_loc = cursor->location;
        if (tmp_loc == 0xd6c0) {
            return 0;
        }

        node_t* new_node = malloc(sizeof(node_t));
        new_node->location = tmp_loc + 64;

        // initalize new data to 0
        for (int i = 0; i < HBANK_SIZE; i++) {
            new_node->data[i] = 0;
        }
        new_node->next = NULL;

        cursor->next = new_node;
        cursor = new_node;
    }

    return location;
}

void heap_delete(node_t** head, node_t* n) {
    // node_t* cursor = *head;
}

void heap_free(node_t* head) {
    node_t* cursor = head;
    while (cursor) {
        node_t* tmp = cursor->next;
        free(cursor);
        cursor = tmp;
    }
}

unsigned int mask(INSTRUCTION n, int i, int j) {
    // Return the ith to the jth bits of an INSTRUCTION
    int p = j - i++ + 1;
    return (((1 << p) - 1) & (n >> (i - 1)));
}

void get_instructions(char *filepath,
                      uint8_t *instructions,
                      uint8_t *data_mem,
                      node_t* heap_bank) {

   int fd;

    // If file can't be read
    if ((fd = open(filepath, O_RDONLY)) < 0) {
        printf("Please provide a valid file path\n");
        heap_free(heap_bank);
        exit(2);
    }

    read(fd, instructions, INST_MEM_SIZE);
    read(fd, data_mem, DATA_MEM_SIZE);
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
        if (debug) printf("sll: r[%d] = r%d(%u) << r%d(%u) = %d", rd, rs1, reg[rs1], rs2, reg[rs2], reg[rs1] << reg[rs2]);
        reg[rd] = reg[rs1] << reg[rs2];
    }
    else if (func3 == 0b101 && func7 == 0b0000000) {  // srl
        if (debug) printf("srl: r[%d] = r%d(%u) >> r%d(%u)", rd, rs1, reg[rs1], rs2, reg[rs2]);
        reg[rd] = reg[rs1] >> reg[rs2];
    }
    else if (func3 == 0b101 && func7 == 0b0100000) {  // sra
        if (debug) printf("sra: r[%d] = r%d(%d) >> r%d(%d)", rd, rs1, reg[rs1], rs2, reg[rs2]);
        uint32_t runoff = mask(reg[rs1], 0, reg[rs2]) << (32 - reg[rs2]);
        reg[rd] = reg[rs1] >> reg[rs2];
        reg[rd] = runoff & reg[rd];
    }
    else if (func3 == 0b010 && func7 == 0b0000000) {  // slt
        if (debug) printf("slt: r[%d] = r%d(%d) < r%d(%d) ? 1 : 0", rd, rs1, reg[rs1], rs2, reg[rs2]);
        reg[rd] = ((int32_t)reg[rs1] < (int32_t)reg[rs2]) ? 1 : 0;
    }
    else if (func3 == 0b011 && func7 == 0b0000000) {  // sltu
        if (debug) printf("sltu: r[%d] = r%d(%d) < r%d(%d) ? 1 : 0", rd, rs1, reg[rs1], rs2, reg[rs2]);
        reg[rd] = ((uint32_t)reg[rs1] <
            (uint32_t)reg[rs2]) ?
            1 :
            0;
    } else {
        printf("Type R, operation not recognised\n");
        printf("func3: %d, func7: %d\n", func3, func7);
    }
}

void i(INSTRUCTION instruction,
       uint8_t instruction_mem[INST_MEM_SIZE],
       uint8_t data_mem[DATA_MEM_SIZE],
       node_t* heap) {

    uint8_t opcode = mask(instruction, 0, 6);
    unsigned int rd = mask(instruction, 7, 11);
    unsigned int func3 = mask(instruction, 12, 14);
    unsigned int rs1 = mask(instruction, 15, 19);
    uint32_t imm = mask(instruction, 20, 31);

    uint32_t unsigned_imm = imm;

    // Sign the immediate
    if ((imm >> 11) & 1) {
        imm = imm | 0b11111111111111111111100000000000;
    }

    if (opcode == 0b0010011) {
        if (func3 == 0b000) {  // addi
            if (debug) printf("addi: r[%d] = r%d(%d) + (%d)", rd, rs1, reg[rs1], imm);
            reg[rd] = reg[rs1] + imm;
        } else if (func3 == 0b100) {  // xori
            if (debug) printf("xori: r[%d] = r%d(%d) ^ (%d)", rd, rs1, reg[rs1], imm);
            reg[rd] = reg[rs1] ^ imm;
            printf("xori");
        } else if (func3 == 0b110) {  // ori
            if (debug) printf("andi: r[%d] = r%d(%d) | (%d)", rd, rs1, reg[rs1], imm);
            reg[rd] = reg[rs1] | imm;
        } else if (func3 == 0b111) {  // andi
            if (debug) printf("andi: r[%d] = r%d(%d) & (%d)", rd, rs1, reg[rs1], imm);
            reg[rd] = reg[rs1] & imm;
        } else if (func3 == 0b010) {  // slti
            if (debug) printf("slti: r[%d] = r%d(%d) < (%d) ? 1 : 0", rd, rs1, reg[rs1], imm);
            reg[rd] = ((int32_t)reg[rs1] < (int32_t)imm) ? 1 : 0;
        } else if (func3 == 0b011) {  // sltiu
            if (debug) printf("sltiu: r[%d] = r%d(%d) < (%d) ? 1 : 0", (uint32_t)rd, rs1, reg[rs1], unsigned_imm);
            reg[rd] = ((uint32_t)reg[rs1] < unsigned_imm) ? 1 : 0;
        }
    } else if (opcode == 0b1100111) {
        if (func3 == 0b000) {  // jalr
            if (debug) printf("jalr: r[%d] = PC(%d)+4; PC=(r%d(%d)+%d)", rd, pc, rs1, reg[rs1], imm);
            reg[rd] = pc + 4;
            pc = reg[rs1] + imm - 4;
        }
    } else if (opcode == 0b0000011) {  // memory loading
        int addy = (reg[rs1] + imm);

        // Load virtual routines
        int read_c = 2066;
        int read_i = 2070;

        if (addy == read_i) {
            int32_t input;
            if (debug) printf("Enter integer: ");
            scanf("%d", &input);
            if (debug) printf("r%d(%d) = %d", rd, reg[rd], input);
            reg[rd] = input;
        } else if (addy == read_c) {
            char input;
            if (debug) printf("Enter character: ");
            scanf("%c", &input);
            if (debug) printf("r%d(%d) = %c", rd, reg[rd], input);
            reg[rd] = input;
        } else {

            uint8_t *location = data_mem;

            if (addy < 0x400) {
                location = instruction_mem;
            } else {
                addy = addy - 0x400;
            }

            if (addy < 0 || addy > DATA_MEM_SIZE) {
                printf("Address out of bounds on load instruction");
                printf("\naddy = %d = r%d(%d) + %d\n", addy, rs1, reg[rs1], imm);
                register_dump();
                heap_free(heap);
                exit(4);
            }

            if (func3 == 0b000) {  // lb
                if (debug) {
                    printf("lb: ");
                    if (location == data_mem) printf("r[%d] = data_mem[r%d(%d) + %d = %d)] = %d", rd, rs1, reg[rs1], imm, reg[rs1]+imm, location[addy]);
                    else printf("r[%d] = inst_mem[r%d(%d) + %d = %d)] = %d", rd, rs1, reg[rs1], imm, reg[rs1]+imm, location[addy]);
                }
                // sext
                uint32_t byte = location[addy];
                if (byte >> 8 & 1) {
                    byte = byte | 0b11111111111111111111111100000000;
                }
                reg[rd] = byte;
            } else if (func3 == 0b001) {  // lh
                if (debug) {
                    printf("lh: ");
                    if (location == data_mem) printf("r[%d] = data_mem[r%d(%d) + %d = %d)] = %d", rd, rs1, reg[rs1], imm, reg[rs1]+imm, location[addy]);
                    else printf("r[%d] = inst_mem[r%d(%d) + %d = %d)] = %d", rd, rs1, reg[rs1], imm, reg[rs1]+imm, location[addy]);
                }
                uint32_t byte2 = location[addy] | location[addy+1] << 8;
                if (byte2 >> 16 & 1) {
                    byte2 = byte2 | 0b11111111111111110000000000000000;
                }
                reg[rd] = byte2;
            } else if (func3 == 0b010) {  // lw
                if (debug) {
                    printf("lw: ");
                    if (location == data_mem) printf("r[%d] = data_mem[r%d(%d) + %d = %d)] = %d", rd, rs1, reg[rs1], imm, reg[rs1]+imm, location[addy]);
                    else printf("r[%d] = inst_mem[r%d(%d) + %d = %d)] = %d", rd, rs1, reg[rs1], imm, reg[rs1]+imm, location[addy]);
                }
                reg[rd] = location[addy] |
                    location[addy+1] << 8 |
                    location[addy+2] << 16 |
                    location[addy+3] << 24;
            } else if (func3 == 0b100) {  // lbu
                if (debug) {
                    printf("lbu: ");
                    if (location == data_mem) printf("r[%d] = data_mem[r%d(%d) + %d = %d)] = %d", rd, rs1, reg[rs1], imm, reg[rs1]+imm, location[addy]);
                    else printf("r[%d] = inst_mem[r%d(%d) + %d = %d)] = %d", rd, rs1, reg[rs1], imm, reg[rs1]+imm, location[addy]);
                }
                reg[rd] = location[addy];
            } else if (func3 == 0b101) {  // lhu
                if (debug) {
                    printf("lhu: ");
                    if (location == data_mem) printf("r[%d] = data_mem[r%d(%d) + %d = %d)] = %d", rd, rs1, reg[rs1], imm, reg[rs1]+imm, location[addy]);
                    else printf("r[%d] = inst_mem[r%d(%d) + %d = %d)] = %d", rd, rs1, reg[rs1], imm, reg[rs1]+imm, location[addy]);
                }
                reg[rd] = location[addy] | location[addy] << 8;
            }
        }
    } else {
        printf("Type I operation not recognised\n");
        printf("opcode: %d, func3: %d\n", opcode, func3);
    }
}

void s(INSTRUCTION instruction,
       uint8_t instruction_mem[INST_MEM_SIZE],
       uint8_t data_mem[DATA_MEM_SIZE],
       node_t* heap) {

    uint32_t func3 = mask(instruction, 12, 14);
    uint32_t rs1 = mask(instruction, 15, 19);
    uint32_t rs2 = mask(instruction, 20, 24);

    int imm0to4 = mask(instruction, 7, 11);
    int imm5to11 = mask(instruction, 25, 31);

    int imm = (imm5to11 << 5) | imm0to4;
    // sign the immediate
    if ((imm >> 11) & 1) {
        imm = imm | 0b11111111111111111111100000000000;
    }

    // Store virtual routines
    int write_c = 0x800;
    int write_i = 0x804;
    int write_ui = 0x808;
    int halt = 0x80c;
    int dump_pc = 0x820;
    int dump_gpr = 0x824;
    int malloc = 0x830;
    int free = 0x834;

    int addy = (reg[rs1] + imm);

    if (addy == halt) {
        printf("CPU Halt Requested\n");
        if (debug) register_dump();
        heap_free(heap);
        exit(0);
    }

    if (addy == write_c) {
        uint8_t b = mask(reg[rs2], 0, 7);
        if (debug) printf("write character from r%d(%d): ", rs2, reg[rs2]);
        printf("%c", b);
        return;
    } else if (addy == write_i) {
        int32_t b = reg[rs2];
        if (debug) printf("write integer from r%d(%d): ", rs2, reg[rs2]);
        printf("%d", b);
        return;
    } else if (addy == write_ui) {
        uint32_t b = reg[rs2];
        if (debug) printf("write unsigned int from r%d(%u): ", rs2, (unsigned)reg[rs2]);
        printf("%x", b);
        return;
    } else if (addy == dump_pc) {
        if (debug) printf("dump program counter: ");
        printf("%x", pc);
    } else if (addy == dump_gpr) {
        if (debug) printf("dump registers: \n");
        register_dump();
    } else if (addy == malloc || addy == free) {
        if (addy == malloc) {
            int size = reg[rs2];
            int location = heap_add(&heap, size);
            reg[28] = location;
            if (debug) printf("malloc size: r%d(%d), r[28] = %x", rs2, reg[rs2], location);
        } else {
            printf("free\n");
            // TODO free memory at reg[rs2]
        }
    } else {

        uint8_t *location = data_mem;

        int heap_flag = 0;

        if (addy < 0x400) {
            location = instruction_mem;
        } else if (addy >= 0x400 && addy < 0x800){
            addy = addy - 0x400;
        } else if (addy >= 0xb700) {
            heap_flag = 1;
            addy = addy - 0xb700;
        } else {
            // TODO
            printf("address store error message\n");
            heap_free(heap);
        }

        if (func3 == 0b000) {  // sb
            if (debug) {
                printf("sb: ");
            }
            uint8_t low8bits = mask(reg[rs2], 0, 7);
            if (heap_flag) {
                int offset = addy % 64;
                int heap_location = (addy / 64) * 64;
                heap_location += 0xb700;
                if (debug) printf("sb: store instruction to heap location: %x, hbank: %x, offset: %d\n", addy, heap_location, offset);

                node_t* cursor = heap;

                while (cursor->location != heap_location || cursor->next != NULL) {
                    cursor = cursor->next;
                }

                cursor->data[offset+0] = low8bits;
            } else {
                if (debug) {
                    if (location == data_mem) printf("data_mem[r%d(%d) + %d = %d] = r%d(%d)", rs1, reg[rs1], imm, addy, rs2, reg[rs2]);
                    else printf("inst_mem[r%d(%d) + %d = %d] = r%d(%d)", rs1, reg[rs1], imm, addy, rs2, reg[rs2]);
                }

                location[addy] = low8bits;
            }
        } else if (func3 == 0b001) {  // sh
            if (debug) {
                printf("sh: ");
            }
            uint8_t low8bits = mask(reg[rs2], 0, 7);
            uint8_t low16bits = mask(reg[rs2], 8, 15);
            if (heap_flag) {
                int offset = addy % 64;
                int heap_location = (addy / 64) * 64;
                heap_location += 0xb700;
                if (debug) printf("store instruction to heap location: %x, hbank: %x, offset: %d\n", addy, heap_location, offset);

                node_t* cursor = heap;

                while (cursor->location != heap_location || cursor->next != NULL) {
                    cursor = cursor->next;
                }

                cursor->data[offset+0] = low8bits;
                cursor->data[offset+1] = low16bits;

            } else {

                if (debug) {
                    if (location == data_mem) printf("data_mem[r%d(%d) + %d = %d] = r%d(%d)", rs1, reg[rs1], imm, addy, rs2, reg[rs2]);
                    else printf("inst_mem[r%d(%d) + %d = %d] = r%d(%d)", rs1, reg[rs1], imm, addy, rs2, reg[rs2]);
                }
                location[addy+0] = low8bits;
                location[addy+1] = low16bits;
            }

        } else if (func3 == 0b010) {  // sw
            if (debug) {
                printf("sw: ");
            }

            uint8_t low8bits = mask(reg[rs2], 0, 7);
            uint8_t low16bits = mask(reg[rs2], 8, 15);
            uint8_t low24bits = mask(reg[rs2], 16, 23);
            uint8_t low32bits = mask(reg[rs2], 24, 31);

            if (heap_flag) {
                int offset = addy % 64;
                int heap_location = (addy / 64) * 64;
                heap_location += 0xb700;
                if (debug) printf("sw: store instruction to heap location: %x, hbank: %x, offset: %d\n", addy, heap_location, offset);

                node_t* cursor = heap;

                while (cursor->location != heap_location || cursor->next != NULL) {
                    cursor = cursor->next;
                }

                cursor->data[offset+0] = low8bits;
                cursor->data[offset+1] = low16bits;
                cursor->data[offset+2] = low24bits;
                cursor->data[offset+3] = low32bits;
            } else {
                if (debug) {
                    if (location == data_mem) printf("data_mem[r%d(%d) + %d = %d] = r%d(%d)", rs1, reg[rs1], imm, addy, rs2, reg[rs2]);
                    else printf("inst_mem[r%d(%d) + %d = %d] = r%d(%d)", rs1, reg[rs1], imm, addy, rs2, reg[rs2]);
                }

                location[addy+0] = low8bits;
                location[addy+1] = low16bits;
                location[addy+2] = low24bits;
                location[addy+3] = low32bits;
            }
        } else {
            printf("Type S operation not found\n");
            printf("func3: %d", func3);
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
        imm = imm | 0b11111111111111111111100000000000;
    }

    if (func3 == 0b000) {  // beq
        if (debug) printf("if (r%d(%d) == r%d(%d)) beq: PC = PC(%d) + %d", rs1, reg[rs1], rs2, reg[rs2], pc, imm << 1);
        if ((int32_t)reg[rs1] == (int32_t)reg[rs2]) {
            pc = pc + (imm << 1) - 4;
        }
    } else if (func3 == 0b001) {  // bne
        if (debug) printf("if (r%d(%d) != r%d(%d)) bne: PC = PC(%d) + %d", rs1, reg[rs1], rs2, reg[rs2], pc, imm << 1);
        if ((int32_t)reg[rs1] != (int32_t)reg[rs2]) {
            pc = pc + (imm << 1) - 4;
        }
    } else if (func3 == 0b100) {  // blt
        if (debug) printf("if (r%d(%d) < r%d(%d)) blt: PC = PC(%d) + %d", rs1, reg[rs1], rs2, reg[rs2], pc, imm << 1);
        if ((int32_t)reg[rs1] < (int32_t)reg[rs2]) {
            pc = pc + (imm << 1) - 4;
        }
    } else if (func3 == 0b110) {  // bltu
        if (debug) printf("if (r%d(%d) < r%d(%d)) bltu: PC = PC(%d) + %d", rs1, (uint32_t)reg[rs1], rs2, (uint32_t)reg[rs2], pc, imm << 1);
        if ((uint32_t)reg[rs1] < (uint32_t)reg[rs2]) {
            pc = pc + (imm << 1) - 4;
        }
    } else if (func3 == 0b101) {  // bge
        if (debug) printf("if (r%d(%d) >= r%d(%d)) bge: PC = PC(%d) + %d", rs1, reg[rs1], rs2, reg[rs2], pc, imm << 1);
        if ((int32_t)reg[rs1] >= (int32_t)reg[rs2]) {
            pc = pc + (imm << 1) - 4;
        }
    } else if (func3 == 0b111) {  // bgeu
        if (debug) printf("if (r%d(%d) >= r%d(%d)) bltu: PC = PC(%d) + %d", rs1, (uint32_t)reg[rs1], rs2, (uint32_t)reg[rs2], pc, imm << 1);
        if ((uint32_t)reg[rs1] >= (uint32_t)reg[rs2]) {
            pc = pc + (imm << 1) - 4;
        }
    } else {
        printf("Type SB, invalid operation\n");
        printf("func3: %d", func3);
    }
}

void u(INSTRUCTION instruction) {
    // lui
    unsigned int rd = mask(instruction, 7, 11);
    uint32_t imm = mask(instruction, 12, 31);

    // bit shift the immediate
    imm = (imm << 12);

    reg[rd] = imm;
    if (debug) printf("lui: r[%d] = %d", rd, imm);
}

void uj(INSTRUCTION instruction) {
    // jal
    unsigned int rd = mask(instruction, 7, 11);

    // bit fuckery
    uint32_t imm20 = mask(instruction, 31, 31);
    uint32_t imm1to10 = mask(instruction, 21, 30);
    uint32_t imm11 = mask(instruction, 20, 20);
    uint32_t imm12to19 = mask(instruction, 12, 19);

    // bit shift and logical 'or'ing all them together
    uint32_t imm = (imm20 << 19) |
        (imm12to19 << 11) |
        (imm11 << 10) |
        imm1to10;

    // sign the immediate
    if ((imm >> 19) & 1) {
        imm = imm | 0b11111111111100000000000000000000;
    }

    if (debug) printf("jal: r[%d] = %d + 4; pc = %d + %d", rd, pc, pc, imm << 1);
    reg[rd] = pc + 4;
    pc = pc + (imm<<1) - 4;
}

void process_instruction(uint8_t instructions[INST_MEM_SIZE],
                         uint8_t byte4,
                         uint8_t byte3,
                         uint8_t byte2,
                         uint8_t byte1,
                         uint8_t data_mem[DATA_MEM_SIZE],
                         node_t* heap) {

    INSTRUCTION instruction = byte1 << 24 |
        byte2 << 16 |
        byte3 << 8 |
        byte4;

    unsigned int opcode = mask(instruction, 0, 6);

    switch (opcode) {
        case R:
            r(instruction);
            break;
        case I:
        case 3:  // memory load
        case 103:  // jalr
            i(instruction, instructions, data_mem, heap);
            break;
        case S:
            s(instruction, instructions, data_mem, heap);
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
            printf("Instruction Not Implemented: 0x%08x\n", instruction);
            printf("PC = 0x%08x;\n", pc);
            register_dump();
            heap_free(heap);
            exit(3);
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

    // defining the head of the heap, not an actual value of the heap
    node_t *heap = heap_init();

    uint8_t instructions[INST_MEM_SIZE] = { 0 };
    uint8_t data_mem[DATA_MEM_SIZE] = { 0 };

    get_instructions(argv[1], instructions, data_mem, heap);

    // Main program loop
    for ( ; pc < INST_MEM_SIZE; pc+=4) {
        if (debug) printf("pc: %04d, ", pc);
        process_instruction(instructions,
                            instructions[pc],
                            instructions[pc+1],
                            instructions[pc+2],
                            instructions[pc+3],
                            data_mem,
                            heap);
        if (debug) printf("\n");
    }

    heap_free(heap);

    return 0;
}
