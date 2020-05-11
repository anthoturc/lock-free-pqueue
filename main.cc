#include "lock-free-pqueue.h"

#include <thread>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <algorithm>
#include <cassert>

typedef std::pair<int, int *> pii;

typedef std::vector<pii> vpii;

typedef std::priority_queue<int, std::vector<int>, std::greater<int>> pqi;

void getRandomVals(vpii&, pqi&, PQueue&, int *, int);



#define MIN_KEY 1
#define MAX_KEY 100

#define MIN_VAL 200
#define MAX_VAL 300

int
main()
{
	static_assert(sizeof(PQLink) == sizeof(uintptr_t));
	static_assert(sizeof(PQVLink) == sizeof(uintptr_t));
	
	pqi stlPQ;
	PQueue myPQ;

	int val1 = 10,
		val2 = 20,
		val3 = 30;
	myPQ.push(0, &val1);
	myPQ.push(1, &val2);
	myPQ.push(2, &val3);
	myPQ.debugPrint();
	// int *res = myPQ.pop();
	// std::cout << *res << std::endl;

	// srand(1); // 1 is the seed 
	// int nRand = 25;
	// int vals[nRand];
	// vpii pairs(nRand);
	

	// getRandomVals(pairs, stlPQ, myPQ, vals, nRand);

	// // for (int i = 0; i < nRand; ++i) {
	// // 	std::cout << pairs[i].first << " -> " << *(pairs[i].second) << "\n";
	// // }

	// for (int i = 0; i < nRand; ++i) {
	// 	assert(stlPQ.top() == *(myPQ.pop()));
	// 	stlPQ.pop();
	// }

	// // for (int i)


	return 0;
}

void
getRandomVals(vpii& pairs, pqi& stlPQ, PQueue& myPQ, int * vals, int nRand)
{

	for (int i = 0; i < nRand; ++i) {
		int key = (rand() % (MAX_KEY - MIN_KEY + 1)) + MIN_KEY;
		vals[i] = (rand() % (MAX_VAL - MIN_VAL + 1)) + MIN_VAL;
		pairs[i] = { key, vals + i };
		myPQ.push(key, vals + i);
		stlPQ.push(vals[i]);
	}
}

// seed for test inputs should be the same

// look into thread safe shared pointers (using atomic operations which take care of reference counting)