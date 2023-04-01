TARGET = vm_riskxvii

CC = gcc

CFLAGS     = -c -Os -s -ffreestanding -Wl,--file-alignment,16,--section-alignment,16
BLOAT      = -c -Wall -Wvla -Werror -O0 -g -std=c11 -Os -ffreestanding -Wl,--file-alignment,16,--section-alignment,16
TEST	   = -fprofile-arcs -ftest-coverage
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


clean:
	rm -f *.o *.obj *.gcov *.gcno *.gcda $(TARGET)

run:
	./$(TARGET)

test:$(TARGET) clean
	$(CC) $(TEST) $(SRC) -o $(TARGET)
	

run_test:
	bash run_tests.sh
