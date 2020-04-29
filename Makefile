CC=g++
CFLAGS=-std=c++17 -Wall -pedantic

ds-test : lock-free-pqueue.cpp
	$(CC) $(CFLAGS) -o $@ $^


.PHONY : clean
clean :
	rm -f ds-test *.o 
