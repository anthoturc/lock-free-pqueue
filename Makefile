CC=g++
CFLAGS=-std=c++17 -Wall -pedantic -g

test : main.cc lock-free-pqueue.cc
	$(CC) $(CFLAGS) -o $@ $^


.PHONY : clean
clean :
	rm -f test *.o 
