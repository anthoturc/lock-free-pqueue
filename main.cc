#include "lock-free-pqueue.h"

#include <thread>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <algorithm>
#include <cassert>

typedef std::pair<int, int> pii;

typedef std::vector<pii> vpii;

typedef std::priority_queue<pii, std::vector<pii>, std::greater<pii>> pqi;

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

	// int val1 = 10,
	// 	val2 = 20,
	// 	val3 = 30;
	// myPQ.push(0, &val1);
	// myPQ.push(1, &val2);
	// myPQ.push(2, &val3);
	// myPQ.debugPrint();

	// int *minV = myPQ.pop();
	// std::cout << *minV << std::endl;
	// int *res = myPQ.pop();
	// std::cout << *res << std::endl;

	srand(1); // 1 is the seed 
	// int nRand = 16;
	// int vals[nRand];
	vpii pairs {
		{ 84, 232 },
		{ 78, 212 },
		{ 94, 256 },
		{ 87, 230 },
		{ 50, 294 },
		{ 63, 239 },
		{ 91, 219 },
		{ 64, 291 },
		{ 41, 205 },
		{ 73, 234 },
		{ 12, 258 },
		{ 68, 251 },
		{ 30, 200 },
		{ 23, 271 },
		{ 70, 261 },
	};

	int nRand = pairs.size();
	for (auto p : pairs) {
		myPQ.push(p.first, &(p.second));
		stlPQ.push(p);
	}

	// use a set to ensure unique keys
	// getRandomVals(pairs, stlPQ, myPQ, vals, nRand);

	// for (int i = 0; i < nRand; ++i) {
	// 	std::cout << pairs[i].first << " -> " << *(pairs[i].second) << "\n";
	// } 

	
	myPQ.debugPrint();

	for (int i = 0; i < nRand; ++i) {
		auto stlTop = stlPQ.top();
		stlPQ.pop();
		PQNode *node = myPQ.pop();

		// std::cout << "stl pq top:" << stlPQ.top().first << std::endl;
		// std::cout << "concurrent pq top:" << node->key_ << std::endl;
		assert(stlTop.first == node->key_);
		assert(stlTop.second == *((int *)(node->val_.w & ((uintptr_t)(-1) << 1))));
	}

	// // for (int i)


	return 0;
}

void
getRandomVals(vpii& pairs, pqi& stlPQ, PQueue& myPQ, int * vals, int nRand)
{

	for (int i = 0; i < nRand; ++i) {
		int key = (rand() % (MAX_KEY - MIN_KEY + 1)) + MIN_KEY;
		vals[i] = (rand() % (MAX_VAL - MIN_VAL + 1)) + MIN_VAL;
		pairs[i] = { key, vals[i] };
		myPQ.push(key, vals + i);
		stlPQ.push({ key, vals[i] });

		assert(vals[i] == *(vals + i));
	}
}

// seed for test inputs should be the same

// look into thread safe shared pointers (using atomic operations which take care of reference counting)