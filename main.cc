/*
 *	Benchmarking program to determine performance of 
 *	lock free priority queue compared to stl priority queue
 */

#include <thread>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <algorithm>
#include <cassert>
#include <unordered_set>

#include "CycleTimer.h"
#include "lock-free-pqueue.h"

typedef std::pair<int, int> pii;

typedef std::vector<pii> vpii;

typedef std::priority_queue<pii, std::vector<pii>, std::greater<pii>> pqi;

typedef std::pair<double, double> pdd;

pdd baselineInserts(vpii&);
void getRandKeyValPairs(vpii&, int);
double twoThreadInserts(vpii&);
void workerInsert(PQueue&, vpii&, int, const int, std::vector<double>&);

#define MIN_KEY 1
#define MAX_KEY 500000

#define MIN_VAL 0
#define MAX_VAL 1000000

#define RAND_SEED 11

#define FORMAT_STR "%.4f"

int
main()
{
	static_assert(sizeof(PQLink) == sizeof(uintptr_t));
	static_assert(sizeof(PQVLink) == sizeof(uintptr_t));
	
	srand(RAND_SEED);

	// int val1 = 10,
	// 	val2 = 20,
	// 	val3 = 30;
	// myPQ.push(0, &val1);
	// myPQ.push(1, &val2);
	// myPQ.push(2, &val3);
	// myPQ.debugPrint();

	srand(RAND_SEED); 
	/* N random key val pairs, to be used across all tests */
	int N = 10000;
	vpii pairs(N);
	getRandKeyValPairs(pairs, N);

	pdd baseline = baselineInserts(pairs);

	std::cout << "Single Thread" << "\n";
	std::cout << "STL Priority Queue:\t\t"; 
	printf(FORMAT_STR, baseline.first);
	std::cout << "s\n";
	std::cout << "Lock Free Priority Queue:\t"; 
	printf(FORMAT_STR, baseline.second);
	std::cout << "s\n";


	double total = twoThreadInserts(pairs);

	std::cout << "Two Threads" << "\n";
	std::cout << "Lock Free Priority Queue:\t"; 
	printf(FORMAT_STR, total);
	std::cout << "s\t"; 
	printf(FORMAT_STR, total/std::max(baseline.first, baseline.second));
	std::cout << "X\n";


	// for (int i = 0; i < nRand; ++i) {
	// 	auto stlTop = stlPQ.top();
	// 	stlPQ.pop();
	// 	PQNode *node = myPQ.pop();

	// 	assert(stlTop.first == node->key_);
	// 	assert(stlTop.second == *((int *)(node->val_.w & ((uintptr_t)(-1) << 1))));
	// }

	return 0;
}

void
getRandKeyValPairs(vpii& keyVals, int N)
{
	std::unordered_set<int> uniKey;

	int i = 0;
	while (i < N) {
		int key = (rand() % (MAX_KEY - MIN_KEY + 1)) + MIN_KEY;
		if (uniKey.find(key) != uniKey.end()) continue;
		
		uniKey.insert(key);
		int val = (rand() % (MAX_VAL - MIN_VAL + 1)) + MIN_VAL;		
		keyVals[i] = { key, val };
		++i;
	}
}

pdd
baselineInserts(vpii& keyVals) 
{
	PQueue lockFreePQ;
	pqi stlPQ;

	double lckFreePQTotalTime = 0.f;
	double stlPQTotalTime = 0.f;

	for (pii p : keyVals) {
		double start_time = CycleTimer::currentSeconds();
		lockFreePQ.push(p.first, &(p.second));
		double end_time = CycleTimer::currentSeconds();

		lckFreePQTotalTime += (end_time - start_time);

		start_time = CycleTimer::currentSeconds();
		stlPQ.push(p);
		end_time = CycleTimer::currentSeconds();

		stlPQTotalTime += (end_time - start_time);
	}

	return { stlPQTotalTime, lckFreePQTotalTime };
}

double
twoThreadInserts(vpii& keyVals)
{
	PQueue pq;

	const int nThreads = 2; /* including master thread */
	std::vector<double> res(nThreads);
	std::thread wrkers[nThreads-1];

	for (int i = 0; i < nThreads-1; ++i) {
		/* https://stackoverflow.com/questions/34078208/passing-object-by-reference-to-stdthread-in-c11 */
		wrkers[i] = std::thread(workerInsert, std::ref(pq), std::ref(keyVals), i, nThreads, std::ref(res));
	}

	workerInsert(pq, keyVals, nThreads-1, nThreads, res);

	for (int i = 0; i < nThreads-1; ++i) {
		wrkers[i].join();
	}

	double maxTimeFromThreads = 0.f;
	for (int i = 0; i < nThreads; ++i) {
		maxTimeFromThreads = std::max(maxTimeFromThreads, res[i]);
	}

	return maxTimeFromThreads;
}

void
workerInsert(PQueue& q, vpii& keyVals, int id, int nThreads, std::vector<double>& res)
{
	double totalTime = 0.f;

	int i = id, N = keyVals.size();

	while (i < N) {
		pii keyVal = keyVals[i];
		
		double start_time = CycleTimer::currentSeconds();
		q.push(keyVal.first, &(keyVal.second));
		double end_time = CycleTimer::currentSeconds();

		totalTime += (end_time - start_time);

		i += nThreads;
	}

	res[id] = totalTime;
}

// seed for test inputs should be the same

// look into thread safe shared pointers (using atomic operations which take care of reference counting)