CC=gcc

CFLAGS=-c
CFLAGS2=-no-pie

all: main

main: HT.o HP.o main_example_BF.o
	$(CC) -o main HT.c HP.c main_example_BF.c BF_64.a $(CFLAGS2)
	


clean:
	$(info Cleaning...)
	rm -rf *o main test_*_index.txt
	clear


