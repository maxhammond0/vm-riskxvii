TARGET = vm_riskxvii

CC = gcc

CFLAGS     = -c -Os -s -ffreestanding -Wl,--file-alignment,16,--section-alignment,16 -g3 -gsplit-dwarf
SRC        = vm_riskxvii.c
OBJ        = $(SRC:.c=.o)

all:$(TARGET)

$(TARGET):$(OBJ)
	$(CC) -o $@ $(OBJ)

.SUFFIXES: .c .o

.c.o:
	 $(CC) $(CFLAGS) $<

run:
	./$(TARGET)

test:
	echo what are we testing?!

run_test:
	bash tests.sh

clean:
	rm -f *.o *.obj $(TARGET)
