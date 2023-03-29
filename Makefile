TARGET = vm_riskxvii

CC = gcc

CFLAGS     = -c -Os -s -ffreestanding -Wl,--file-alignment,16,--section-alignment,16
BLOAT      = -c -Wall -Wvla -Werror -O0 -g -std=c11 -Os -ffreestanding -Wl,--file-alignment,16,--section-alignment,16
SRC        = vm_riskxvii.c
OBJ        = $(SRC:.c=.o)

all:$(TARGET) compress

$(TARGET):$(OBJ)
	$(CC) -o $@ $(OBJ)

.SUFFIXES: .c .o

.c.o:
	 $(CC) $(BLOAT) $<

compress:
	strip --strip-all $(TARGET)

run:
	./$(TARGET)

test:
	echo what are we testing?!

run_test:
	bash tests.sh

clean:
	rm -f *.o *.obj $(TARGET)
