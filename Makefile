CC=g++
CFLAGS=-std=c++17 -Wall -pedantic

test : test-serial-skiplist.cpp lock-free-pqueue.cpp
	$(CC) $(CFLAGS) -o $@ $^


.PHONY : clean
clean :
	rm -f test *.o 