CC = g++
CFLAGS = -std=c++11 -Wall -Werror -O2
# CFLAGS = -std=c++11 -Wall -Werror -O2 -Qunused-arguments -pthread

run: sortTester
	./sortTester

sortTester: sortTester.o BucketSort.o
	$(CC) $(CFLAGS) -o sortTester sortTester.o BucketSort.o

sortTester.o: sortTester.cpp BucketSort.h
	$(CC) $(CFLAGS) -c sortTester.cpp

BucketSort.o: BucketSort.h BucketSort.cpp
	$(CC) $(CFLAGS) -c BucketSort.cpp

clean:
	rm -rf *.o sortTester

all: sortTester

tentimes: sortTester
	for i in 1 2 3 4 5 6 7 8 9 10; do ./sortTester; done
