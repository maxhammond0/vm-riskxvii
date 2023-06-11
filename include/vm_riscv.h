#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <limits.h>

#define INST_MEM_SIZE 1024
#define DATA_MEM_SIZE 1024

#define R 51
#define I 19
#define S 35
#define SB 99
#define U 55
#define UJ 111

#define WRITE_C 0x800
#define WRITE_I 0x804
#define WRITE_UI 0x808
#define HALT 0x80c
#define DUMP_PC 0x820
#define DUMP_GPR 0x824
#define MALLOC 0x830
#define FREE 0x834
