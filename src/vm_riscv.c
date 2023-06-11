#include "../include/vm_riscv.h"
#include "../include/hbank.h"

typedef uint32_t INSTRUCTION;
typedef struct node node_t;

#define HBANK_SIZE 64

struct node {
    uint8_t data[HBANK_SIZE];
    uint32_t location;
    node_t* next;
};

// program counter
int pc = 0;

// 0 register and 31 general purpose registers
uint32_t reg[32] = {0};

void register_dump() {
    for (int i = 0; i < 32; i++) {
        printf("R[%d] = 0x%08x;\n", i, reg[i]);
    }
}

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

void heap_delete(node_t** head, int location) {
    node_t* cursor = *head;

    while (cursor->next->location != location) {
        if (cursor->next == NULL) return;
        cursor = cursor->next;
    }
    node_t* future = cursor->next->next;
    free(cursor->next);
    cursor->next = future;
    return;
}

void heap_free(node_t* head) {
    node_t* cursor = head;
    while (cursor) {
        node_t* tmp = cursor->next;
        free(cursor);
        cursor = tmp;
    }
}

node_t* find_node(node_t* heap, int location) {

    node_t* cursor = heap->next;
    while (cursor != NULL) {
        if (cursor->location == location) {
            break;
        }
        cursor = cursor->next;
    }
    return cursor;
}

unsigned int mask(INSTRUCTION n, int i, int j) {
    // Return the ith to the jth bits of an INSTRUCTION
    int p = j - i++ + 1;
    return (((1 << p) - 1) & (n >> (i - 1)));
}

void illegal_op(INSTRUCTION instruction, node_t *heap) {
    heap_free(heap);

    printf("Illegal Operation: 0x%08x\n", instruction);
    printf("PC = 0x%08x;\n", pc);
    register_dump();
    exit(1);
}

void not_implemented(INSTRUCTION instruction, node_t *heap) {
    heap_free(heap);

    printf("Instruction Not Implemented: 0x%08x\n", instruction);
    printf("PC = 0x%08x;\n", pc);
    register_dump();
    exit(1);
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

void r(INSTRUCTION instruction, node_t* heap) {

    unsigned int rd = mask(instruction, 7, 11);
    unsigned int func3 = mask(instruction, 12, 14);
    unsigned int rs1 = mask(instruction, 15, 19);
    unsigned int rs2 = mask(instruction, 20, 24);
    unsigned int func7 = mask(instruction, 25, 31);


    if (func3 == 0b000 && func7 == 0b0000000) {  // add
        #ifdef DEBUG
            printf("add: r[%d] = r%d(%d) + r%d(%d)", rd,rs1, reg[rs1], rs2,
                   reg[rs2]);
        #endif
        reg[rd] = reg[rs1] + reg[rs2];
    }
    else if (func3 == 0b000 && func7 == 0b0100000) {  // sub
        #ifdef DEBUG
            printf("sub: r[%d] = r%d(%d) - r%d(%d)", rd, rs1, reg[rs1], rs2,
                   reg[rs2]);
        #endif
        reg[rd] = reg[rs1] - reg[rs2];
    }
    else if (func3 == 0b100 && func7 == 0b0000000) {  // xor
        #ifdef DEBUG
            printf("xor: r[%d] = r%d(%d) ^ r%d(%d)", rd, rs1, reg[rs1], rs2,
                   reg[rs2]);
        #endif
        reg[rd] = reg[rs1] ^ reg[rs2];
    }
    else if (func3 == 0b110 && func7 == 0b0000000) {  // or
        #ifdef DEBUG
            printf("or: r[%d] = r%d(%d) | r%d(%d)", rd, rs1, reg[rs1], rs2, reg[rs2]);
        #endif
        reg[rd] = reg[rs1] | reg[rs2];
    }
    else if (func3 == 0b111 && func7 == 0b0000000) {  // and
        #ifdef DEBUG
            printf("and: r[%d] = r%d(%d) & r%d(%d)", rd, rs1, reg[rs1], rs2, reg[rs2]);
        #endif
        reg[rd] = reg[rs1] & reg[rs2];
    }
    else if (func3 == 0b001 && func7 == 0b0000000) {  // sll
        #ifdef DEBUG
            printf("sll: r[%d] = r%d(%u) << r%d(%u) = %d", rd, rs1, reg[rs1], rs2, reg[rs2], reg[rs1] << reg[rs2]);
        #endif
        reg[rd] = reg[rs1] << reg[rs2];
    }
    else if (func3 == 0b101 && func7 == 0b0000000) {  // srl
        #ifdef DEBUG
            printf("srl: r[%d] = r%d(%u) >> r%d(%u)", rd, rs1, reg[rs1], rs2, reg[rs2]);
        #endif
        reg[rd] = reg[rs1] >> reg[rs2];
    }
    else if (func3 == 0b101 && func7 == 0b0100000) {  // sra
        #ifdef DEBUG
            printf("sra: r[%d] = r%d(%d) >> r%d(%d)", rd, rs1, reg[rs1], rs2, reg[rs2]);
        #endif
        uint32_t runoff = mask(reg[rs1], 0, reg[rs2]) << (32 - reg[rs2]);
        reg[rd] = reg[rs1] >> reg[rs2];
        reg[rd] = runoff & reg[rd];
    }
    else if (func3 == 0b010 && func7 == 0b0000000) {  // slt
        #ifdef DEBUG
            printf("slt: r[%d] = r%d(%d) < r%d(%d) ? 1 : 0", rd, rs1, reg[rs1], rs2, reg[rs2]);
        #endif
        reg[rd] = ((int32_t)reg[rs1] < (int32_t)reg[rs2]) ? 1 : 0;
    }
    else if (func3 == 0b011 && func7 == 0b0000000) {  // sltu
        #ifdef DEBUG
            printf("sltu: r[%d] = r%d(%d) < r%d(%d) ? 1 : 0", rd, rs1, reg[rs1], rs2, reg[rs2]);
        #endif
        reg[rd] = ((uint32_t)reg[rs1] <
            (uint32_t)reg[rs2]) ?
            1 :
            0;
    } else {
        not_implemented(instruction, heap);
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
            #ifdef DEBUG
                printf("addi: r[%d] = r%d(%d) + (%d)", rd, rs1, reg[rs1], imm);
            #endif
            reg[rd] = reg[rs1] + imm;
        } else if (func3 == 0b100) {  // xori
            #ifdef DEBUG
                printf("xori: r[%d] = r%d(%d) ^ (%d)", rd, rs1, reg[rs1], imm);
            #endif
            reg[rd] = reg[rs1] ^ imm;
        } else if (func3 == 0b110) {  // ori
            #ifdef DEBUG
                printf("andi: r[%d] = r%d(%d) | (%d)", rd, rs1, reg[rs1], imm);
            #endif
            reg[rd] = reg[rs1] | imm;
        } else if (func3 == 0b111) {  // andi
            #ifdef DEBUG
                printf("andi: r[%d] = r%d(%d) & (%d)", rd, rs1, reg[rs1], imm);
            #endif
            reg[rd] = reg[rs1] & imm;
        } else if (func3 == 0b010) {  // slti
            #ifdef DEBUG
                printf("slti: r[%d] = r%d(%d) < (%d) ? 1 : 0", rd, rs1, reg[rs1], imm);
            #endif
            reg[rd] = ((int32_t)reg[rs1] < (int32_t)imm) ? 1 : 0;
        } else if (func3 == 0b011) {  // sltiu
            #ifdef DEBUG
                printf("sltiu: r[%d] = r%d(%d) < (%d) ? 1 : 0", (uint32_t)rd, rs1, reg[rs1], unsigned_imm);
            #endif
            reg[rd] = ((uint32_t)reg[rs1] < unsigned_imm) ? 1 : 0;
        }
    } else if (opcode == 0b1100111) {
        if (func3 == 0b000) {  // jalr
            #ifdef DEBUG
                printf("jalr: r[%d] = PC(%d)+4; PC=(r%d(%d)+%d)", rd, pc, rs1, reg[rs1], imm);
            #endif
            reg[rd] = pc + 4;
            pc = reg[rs1] + imm - 4;
        }
    } else if (opcode == 0b0000011) {  // memory loading
        int address = (reg[rs1] + imm);

        // Load virtual routines
        int read_c = 2066;
        int read_i = 2070;

        if (address == read_i) {
            int32_t input;
            #ifdef DEBUG
                printf("Enter integer: ");
            #endif
            scanf("%d", &input);
            #ifdef DEBUG
                printf("r%d(%d) = %d", rd, reg[rd], input);
            #endif
            reg[rd] = input;
        } else if (address == read_c) {
            char input;
            #ifdef DEBUG
                printf("Enter character: ");
            #endif
            scanf("%c", &input);
            #ifdef DEBUG
                printf("r%d(%d) = %c", rd, reg[rd], input);
            #endif
            reg[rd] = input;
        } else {

            int heap_flag = 0;
            uint8_t *location = data_mem;

            if (address < 0x400) {
                location = instruction_mem;
            } else if (address >= 0x400 && address < 0x800){
                address = address - 0x400;
            } else if (address >= 0xb700) {
                heap_flag = 1;
                address = address - 0xb700;
            } else {
                illegal_op(instruction, heap);
            }

            if (func3 == 0b000) {  // lb
                #ifdef DEBUG
                    printf("lb: ");
                #endif
                uint32_t byte;
                if (heap_flag) {
                    int offset = address % 64;
                    int heap_location = (address / 64) * 64;
                    heap_location += 0xb700;
                    node_t* cursor = find_node(heap, heap_location);
                    #ifdef DEBUG
                        printf("to heap location: %x, hbank: %x, offset: %d", address, heap_location, offset);
                    #endif

                    if (!cursor) {
                        illegal_op(instruction, heap);
                    }

                    byte = cursor->data[offset];

                } else {
                    #ifdef DEBUG
                        if (location == data_mem) printf("r[%d] = data_mem[r%d(%d) + %d = %d)] = %d", rd, rs1, reg[rs1], imm, reg[rs1]+imm, location[address]);
                        else printf("r[%d] = inst_mem[r%d(%d) + %d = %d)] = %d", rd, rs1, reg[rs1], imm, reg[rs1]+imm, location[address]);
                    #endif
                    byte = location[address];
                }
                if (byte >> 8 & 1) {
                    byte = byte | 0b11111111111111111111111100000000;
                }
                reg[rd] = byte;
            } else if (func3 == 0b001) {  // lh
                #ifdef DEBUG
                    printf("lh: ");
                #endif
                uint32_t byte2;
                if (heap_flag) {
                    int offset = address % 64;
                    int heap_location = (address / 64) * 64;
                    heap_location += 0xb700;
                    #ifdef DEBUG
                        printf("to heap location: %x, hbank: %x, offset: %d", address, heap_location, offset);
                    #endif


                    node_t* cursor = find_node(heap, heap_location);

                    if (!cursor) {
                        illegal_op(instruction, heap);
                    }

                    byte2 = cursor->data[offset] | cursor->data[offset+1] << 8;

                } else {
                    #ifdef DEBUG
                        if (location == data_mem) printf("r[%d] = data_mem[r%d(%d) + %d = %d)] = %d", rd, rs1, reg[rs1], imm, reg[rs1]+imm, location[address]);
                        else printf("r[%d] = inst_mem[r%d(%d) + %d = %d)] = %d", rd, rs1, reg[rs1], imm, reg[rs1]+imm, location[address]);
                    #endif
                    byte2 = location[address] | location[address+1] << 8;
                }

                if (byte2 >> 16 & 1) {
                    byte2 = byte2 | 0b11111111111111110000000000000000;
                }
                reg[rd] = byte2;
            } else if (func3 == 0b010) {  // lw
                #ifdef DEBUG
                printf("lw: ");
                #endif
                if (heap_flag) {
                    int offset = address % 64;
                    int heap_location = (address / 64) * 64;
                    heap_location += 0xb700;
                    #ifdef DEBUG
                        printf("to heap location: %x, hbank: %x, offset: %d", address, heap_location, offset);
                    #endif

                    node_t* cursor = find_node(heap, heap_location);

                    if (!cursor) {
                        illegal_op(instruction, heap);
                    }

                    reg[rd] = cursor->data[offset] |
                        cursor->data[offset+1] << 8 |
                        cursor->data[offset+2] << 16 |
                        cursor->data[offset+3] << 24;

                } else {
                    #ifdef DEBUG
                        if (location == data_mem) printf("r[%d] = data_mem[r%d(%d) + %d = %d)] = %d", rd, rs1, reg[rs1], imm, reg[rs1]+imm, location[address]);
                        else printf("r[%d] = inst_mem[r%d(%d) + %d = %d)] = %d", rd, rs1, reg[rs1], imm, reg[rs1]+imm, location[address]);
                    #endif
                    reg[rd] = location[address] |
                        location[address+1] << 8 |
                        location[address+2] << 16 |
                        location[address+3] << 24;
                }
            } else if (func3 == 0b100) {  // lbu
                #ifdef DEBUG
                    printf("lbu: ");
                #endif
                if (heap_flag) {
                    int offset = address % 64;
                    int heap_location = (address / 64) * 64;
                    heap_location += 0xb700;
                    #ifdef DEBUG
                        printf("to heap location: %x, hbank: %x, offset: %d", address, heap_location, offset);
                    #endif

                    node_t* cursor = find_node(heap, heap_location);

                    if (!cursor) {
                        illegal_op(instruction, heap);
                    }

                    reg[rd] = cursor->data[offset];
                } else {
                    #ifdef DEBUG
                        if (location == data_mem) printf("r[%d] = data_mem[r%d(%d) + %d = %d)] = %d", rd, rs1, reg[rs1], imm, reg[rs1]+imm, location[address]);
                        else printf("r[%d] = inst_mem[r%d(%d) + %d = %d)] = %d", rd, rs1, reg[rs1], imm, reg[rs1]+imm, location[address]);
                    #endif
                    reg[rd] = location[address];
                }
            } else if (func3 == 0b101) {  // lhu
                #ifdef DEBUG
                    printf("lhu: ");
                #endif
                if (heap_flag) {
                    int offset = address % 64;
                    int heap_location = (address / 64) * 64;
                    heap_location += 0xb700;
                    #ifdef DEBUG
                        printf("to heap location: %x, hbank: %x, offset: %d", address, heap_location, offset);
                    #endif

                    node_t* cursor = find_node(heap, heap_location);

                    if (!cursor) {
                        illegal_op(instruction, heap);
                    }
                    reg[rd] = cursor->data[offset] |
                        cursor->data[offset+1] << 8;

                } else {
                    #ifdef DEBUG
                        if (location == data_mem) printf("r[%d] = data_mem[r%d(%d) + %d = %d)] = %d", rd, rs1, reg[rs1], imm, reg[rs1]+imm, location[address]);
                        else printf("r[%d] = inst_mem[r%d(%d) + %d = %d)] = %d", rd, rs1, reg[rs1], imm, reg[rs1]+imm, location[address]);
                    #endif
                    reg[rd] = location[address] | location[address+1] << 8;
                }
            }
        }
    } else {
        not_implemented(instruction, heap);
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

    int address = (reg[rs1] + imm);

    if (address == HALT) {
        printf("CPU Halt Requested\n");
        #ifdef DEBUG
            register_dump();
        #endif
        heap_free(heap);
        exit(0);
    }

    if (address == WRITE_C) {
        uint8_t b = mask(reg[rs2], 0, 7);
        #ifdef DEBUG
            printf("write character from r%d(%d): ", rs2, reg[rs2]);
        #endif
        printf("%c", b);
        return;
    } else if (address == WRITE_I) {
        int32_t b = reg[rs2];
        #ifdef DEBUG
            printf("write integer from r%d(%d): ", rs2, reg[rs2]);
        #endif
        printf("%d", b);
        return;
    } else if (address == WRITE_UI) {
        uint32_t b = reg[rs2];
        #ifdef DEBUG
            printf("write unsigned int from r%d(%u): ", rs2, (unsigned)reg[rs2]);
        #endif
        printf("%x", b);
        return;
    } else if (address == DUMP_PC) {
        #ifdef DEBUG
            printf("dump program counter: ");
        #endif
        printf("%x", pc);
    } else if (address == DUMP_GPR) {
        #ifdef DEBUG
            printf("dump registers: \n");
        #endif
        register_dump();
    } else if (address == MALLOC || address == FREE) {
        if (address == MALLOC) {
            int size = reg[rs2];
            int location = heap_add(&heap, size);
            reg[28] = location;
            #ifdef DEBUG
                printf("malloc size: r%d(%d), r[28] = %x", rs2, reg[rs2], location);
            #endif
        } else {
            int location = reg[rs2];
            #ifdef DEBUG
                printf("free: r%d(%d)", rs2, reg[rs2]);
            #endif

            node_t* cursor = heap->next;
            while (cursor != NULL) {
                if (cursor->location == location) {
                    heap_delete(&heap, location);
                    break;
                }
                cursor = cursor->next;
            }
            if (!cursor) {
                illegal_op(instruction, heap);
            }
        }
    } else {

        uint8_t *location = data_mem;

        int heap_flag = 0;

        if (address >= 0x400 && address < 0x800){
            address = address - 0x400;
        } else if (address >= 0xb700) {
            heap_flag = 1;
            address = address - 0xb700;
        } else {
            illegal_op(instruction, heap);
        }

        if (func3 == 0b000) {  // sb
            uint8_t low8bits = mask(reg[rs2], 0, 7);
            if (heap_flag) {
                int offset = address % 64;
                int heap_location = (address / 64) * 64;
                heap_location += 0xb700;
                #ifdef DEBUG
                    printf("sb: store instruction to heap location: %x, hbank: %x, offset: %d", address, heap_location, offset);
                #endif

                node_t* cursor = find_node(heap, heap_location);

                if (!cursor) {
                    illegal_op(instruction, heap);
                }

                cursor->data[offset+0] = low8bits;
            } else {
                #ifdef DEBUG
                    if (location == data_mem) printf("data_mem[r%d(%d) + %d = %d] = r%d(%d)", rs1, reg[rs1], imm, address, rs2, reg[rs2]);
                    else printf("inst_mem[r%d(%d) + %d = %d] = r%d(%d)", rs1, reg[rs1], imm, address, rs2, reg[rs2]);
                #endif

                location[address] = low8bits;
            }
        } else if (func3 == 0b001) {  // sh
            #ifdef DEBUG
                printf("sh: ");
            #endif
            uint8_t low8bits = mask(reg[rs2], 0, 7);
            uint8_t low16bits = mask(reg[rs2], 8, 15);
            if (heap_flag) {
                int offset = address % 64;
                int heap_location = (address / 64) * 64;
                heap_location += 0xb700;
                #ifdef DEBUG
                    printf("heap location: %x, hbank: %x, offset: %d", address, heap_location, offset);
                #endif

                node_t* cursor = find_node(heap, heap_location);

                if (!cursor) {
                    illegal_op(instruction, heap);
                }

                cursor->data[offset+0] = low8bits;
                cursor->data[offset+1] = low16bits;

            } else {
                #ifdef DEBUG
                    if (location == data_mem) printf("data_mem[r%d(%d) + %d = %d] = r%d(%d)", rs1, reg[rs1], imm, address, rs2, reg[rs2]);
                    else printf("inst_mem[r%d(%d) + %d = %d] = r%d(%d)", rs1, reg[rs1], imm, address, rs2, reg[rs2]);
                #endif

                location[address+0] = low8bits;
                location[address+1] = low16bits;
            }

        } else if (func3 == 0b010) {  // sw
            #ifdef DEBUG
                printf("sw: ");
            #endif

            uint8_t low8bits = mask(reg[rs2], 0, 7);
            uint8_t low16bits = mask(reg[rs2], 8, 15);
            uint8_t low24bits = mask(reg[rs2], 16, 23);
            uint8_t low32bits = mask(reg[rs2], 24, 31);

            if (heap_flag) {
                int offset = address % 64;
                int heap_location = (address / 64) * 64;
                heap_location += 0xb700;
                #ifdef DEBUG
                    printf("heap location: %x, hbank: %x, offset: %d", address, heap_location, offset);
                #endif

                node_t* cursor = find_node(heap, heap_location);

                if (!cursor) {
                    illegal_op(instruction, heap);
                }

                cursor->data[offset+0] = low8bits;
                cursor->data[offset+1] = low16bits;
                cursor->data[offset+2] = low24bits;
                cursor->data[offset+3] = low32bits;
            } else {
                #ifdef DEBUG
                    if (location == data_mem) printf("data_mem[r%d(%d) + %d = %d] = r%d(%d)", rs1, reg[rs1], imm, address, rs2, reg[rs2]);
                    else printf("inst_mem[r%d(%d) + %d = %d] = r%d(%d)", rs1, reg[rs1], imm, address, rs2, reg[rs2]);
                #endif

                location[address+0] = low8bits;
                location[address+1] = low16bits;
                location[address+2] = low24bits;
                location[address+3] = low32bits;
            }
        } else {
            not_implemented(instruction, heap);
        }
    }
}

void sb(INSTRUCTION instruction, node_t* heap) {

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
        #ifdef DEBUG
            printf("if (r%d(%d) == r%d(%d)) beq: PC = PC(%d) + %d", rs1, reg[rs1], rs2, reg[rs2], pc, imm << 1);
        #endif
        if ((int32_t)reg[rs1] == (int32_t)reg[rs2]) {
            pc = pc + (imm << 1) - 4;
        }
    } else if (func3 == 0b001) {  // bne
        #ifdef DEBUG
            printf("if (r%d(%d) != r%d(%d)) bne: PC = PC(%d) + %d", rs1, reg[rs1], rs2, reg[rs2], pc, imm << 1);
        #endif
        if ((int32_t)reg[rs1] != (int32_t)reg[rs2]) {
            pc = pc + (imm << 1) - 4;
        }
    } else if (func3 == 0b100) {  // blt
        #ifdef DEBUG
            printf("if (r%d(%d) < r%d(%d)) blt: PC = PC(%d) + %d", rs1, reg[rs1], rs2, reg[rs2], pc, imm << 1);
        #endif
        if ((int32_t)reg[rs1] < (int32_t)reg[rs2]) {
            pc = pc + (imm << 1) - 4;
        }
    } else if (func3 == 0b110) {  // bltu
        #ifdef DEBUG
            printf("if (r%d(%d) < r%d(%d)) bltu: PC = PC(%d) + %d", rs1, (uint32_t)reg[rs1], rs2, (uint32_t)reg[rs2], pc, imm << 1);
        #endif
        if ((uint32_t)reg[rs1] < (uint32_t)reg[rs2]) {
            pc = pc + (imm << 1) - 4;
        }
    } else if (func3 == 0b101) {  // bge
        #ifdef DEBUG
        printf("if (r%d(%d) >= r%d(%d)) bge: PC = PC(%d) + %d", rs1, reg[rs1], rs2, reg[rs2], pc, imm << 1);
        #endif
        if ((int32_t)reg[rs1] >= (int32_t)reg[rs2]) {
            pc = pc + (imm << 1) - 4;
        }
    } else if (func3 == 0b111) {  // bgeu
        #ifdef DEBUG
        printf("if (r%d(%d) >= r%d(%d)) bltu: PC = PC(%d) + %d", rs1, (uint32_t)reg[rs1], rs2, (uint32_t)reg[rs2], pc, imm << 1);
        #endif
        if ((uint32_t)reg[rs1] >= (uint32_t)reg[rs2]) {
            pc = pc + (imm << 1) - 4;
        }
    } else {
        not_implemented(instruction, heap);
    }
}

void u(INSTRUCTION instruction) {
    // lui
    unsigned int rd = mask(instruction, 7, 11);
    uint32_t imm = mask(instruction, 12, 31);

    // bit shift the immediate
    imm = (imm << 12);

    reg[rd] = imm;
    #ifdef DEBUG
        printf("lui: r[%d] = %d", rd, imm);
    #endif
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

    #ifdef DEBUG
        printf("jal: r[%d] = %d + 4; pc = %d + %d", rd, pc, pc, imm << 1);
    #endif
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
            r(instruction, heap);
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
            sb(instruction, heap);
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
            not_implemented(instruction, heap);
    }

    reg[0] = 0;
}

int main( int argc, char *argv[]) {

    // Wrong number of cmd line arguments
    if ( argc != 2) {
        printf("Wrong number of command line arguments\n");
        printf("Exiting...\n");
        return 2;
    }

    // defining the head of the heap, not an actual value of the heap
    node_t *heap = heap_init();

    uint8_t instructions[INST_MEM_SIZE] = { 0 };
    uint8_t data_mem[DATA_MEM_SIZE] = { 0 };

    get_instructions(argv[1], instructions, data_mem, heap);

    // Main program loop
    for ( ; pc < INST_MEM_SIZE; pc+=4) {
        #ifdef DEBUG
            printf("pc: %04x, ", pc);
        #endif
        process_instruction(instructions,
                            instructions[pc],
                            instructions[pc+1],
                            instructions[pc+2],
                            instructions[pc+3],
                            data_mem,
                            heap);
        #ifdef DEBUG
            printf("\n");
        #endif
    }

    heap_free(heap);

    return 0;
}
