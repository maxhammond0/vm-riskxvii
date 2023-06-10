TARGET = vm_riscv

CC = gcc

CFLAGS     = -Wall -Wvla -Werror -Os -g
DEBUG      = -Wall -Wvla -Werror -Os -g -DDEBUG
TEST       = -fprofile-arcs -ftest-coverage
SRC        = src/vm_riskxvii.c
# OBJ        = $(SRC:.c=.o)
RM         = rm -f

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

debug:
	$(CC) $(DEBUG) $(SRC) -o $(TARGET)

clean:
	$(RM) *.o *.obj *.gcov *.gcno *.gcda $(TARGET)

run:
	./$(TARGET)

tests:$(TARGET) clean
	$(CC) $(TEST) $(SRC) -o $(TARGET)

run_tests:
	bash run_tests.sh
