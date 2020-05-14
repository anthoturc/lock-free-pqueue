/*
 *	Benchmarking program to determine performance of 
 *	lock free priority queue compared to stl priority queue
 */

#include <thread>
#include <mutex>

#include <iostream>

#include <vector>
#include <queue>
#include <unordered_set>
#include <algorithm>

#include <cassert>
#include <ctime>
#include <cstdlib>

#include "lock-free-pqueue.h"

typedef std::pair<int, int> pii;

typedef std::vector<pii> vpii;

typedef std::priority_queue<pii, std::vector<pii>, std::greater<pii>> pqi;

typedef std::pair<double, double> pdd;


class CoarseGrainPQ
{
public:
	CoarseGrainPQ() {}
	void push(int key, int val);
	int size();
private:
	pqi q_;
	std::mutex mtx_;
};

void
CoarseGrainPQ::push(int key, int val)
{
	mtx_.lock();
	q_.push({ key, val });
	mtx_.unlock();
}

int
CoarseGrainPQ::size()
{
	mtx_.lock();
	int sz = q_.size();
	mtx_.unlock();
	return sz;
}

void PrintUsage();
pdd SingleThreadBaseline(vpii&);
void GetRandKeyValPairs(vpii&);
pdd MultiThreadInserts(vpii&, int);
void LockFreePQWorkerInsert(PQueue&, vpii&, int, int);
void STLPQWorkerInsert(CoarseGrainPQ&, vpii&, int, int);

#define MIN_KEY 1
#define MAX_KEY 500000

#define MIN_VAL 0
#define MAX_VAL 1000000

#define RAND_SEED 11

#define FORMAT_STR "%.4f"

/* number of elements to insert */
const int N = 1000000;

int
main(int argc, char **argv)
{

	if (argc != 2) {
		PrintUsage();
		return 1;
	}

	/* second argument of argv should be number of threads */
	int T = atoi(argv[1]);

	static_assert(sizeof(PQLink) == sizeof(uintptr_t));
	static_assert(sizeof(PQVLink) == sizeof(uintptr_t));
	
	/* N random key val pairs, to be used across all tests */
	srand(RAND_SEED);

	vpii pairs(N);
	GetRandKeyValPairs(pairs);

	/* single threaded baseline */
	pdd baseline = SingleThreadBaseline(pairs);
	
	double stlPQTimeSingleThread = baseline.first,
		lckFreePQTimeSingleThread = baseline.second;

	pdd totals = MultiThreadInserts(pairs, T);

	double stlPQTotalTimeMultiThread = totals.first,
		lckFreePQTimeMultiThread = totals.second;

	std::cout << "Thread(s): " << T << "\n";
	std::cout << "STL Priority Queue:\t\t"; 
	printf(FORMAT_STR, stlPQTotalTimeMultiThread);
	std::cout << "s\t\t";
	printf(FORMAT_STR, stlPQTotalTimeMultiThread > 0 ? stlPQTimeSingleThread/stlPQTotalTimeMultiThread : 1.f);
	std::cout << "X\n";
	std::cout << "Lock Free Priority Queue:\t"; 
	printf(FORMAT_STR, lckFreePQTimeMultiThread);
	std::cout << "s\t\t";
	printf(FORMAT_STR, lckFreePQTimeSingleThread/lckFreePQTimeMultiThread);
	std::cout << "X\n";

	return 0;
}

void
GetRandKeyValPairs(vpii& keyVals)
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
SingleThreadBaseline(vpii& keyVals) 
{
	PQueue lockFreePQ;
	CoarseGrainPQ stlPQ;

	double lckFreePQTotalTime = 0.f;
	double stlPQTotalTime = 0.f;

	clock_t start_time = clock();
	for (pii p : keyVals) {
		lockFreePQ.push(p.first, &(p.second));
	}
	clock_t end_time = clock();

	lckFreePQTotalTime = (end_time - start_time)/(double)CLOCKS_PER_SEC;

	start_time = clock();
	for (pii p : keyVals) {
		stlPQ.push(p.first, p.second);
	}
	end_time = clock();	

	stlPQTotalTime = (end_time - start_time)/(double)CLOCKS_PER_SEC;
	

	/* ensure the stlPQ has N elements */
	assert(stlPQ.size() == N);

	return { stlPQTotalTime, lckFreePQTotalTime };
}

pdd
MultiThreadInserts(vpii& keyVals, int T)
{
	PQueue pQ;
	CoarseGrainPQ stlPQ;

	std::vector<std::thread> wrkers(T-1);
	
	clock_t start_time = clock();

	/* run lock free pq inserts */
	for (int i = 0; i < T-1; ++i) {
		/* https://stackoverflow.com/questions/34078208/passing-object-by-reference-to-stdthread-in-c11 */
		wrkers[i] = std::thread(LockFreePQWorkerInsert, std::ref(pQ), std::ref(keyVals), i, T);
	}

	LockFreePQWorkerInsert(pQ, keyVals, T-1, T);
	
	for (int i = 0; i < T-1; ++i) {
		wrkers[i].join();
	}
	clock_t end_time = clock();

	double lockFreePQTotalTime = (end_time - start_time)/(double)CLOCKS_PER_SEC;

	start_time = clock();
	/* run coarse grained stl pq inserts */
	for (int i = 0; i < T-1; ++i) {
		wrkers[i] = std::thread(STLPQWorkerInsert, std::ref(stlPQ), std::ref(keyVals), i, T); 
	}

	STLPQWorkerInsert(stlPQ, keyVals, T-1, T);

	for (int i = 0; i < T-1; ++i) {
		wrkers[i].join();
	}
	end_time = clock();

	/* ensure the stlPQ has N elements */
	assert(stlPQ.size() == N);
	
	double stlPQTotalTime = (end_time - start_time)/(double)CLOCKS_PER_SEC;


	return { stlPQTotalTime, lockFreePQTotalTime };
}

void
LockFreePQWorkerInsert(PQueue& q, vpii& keyVals, int id, int T)
{
	int i = id;
	while (i < N) {
		pii keyVal = keyVals[i];
		q.push(keyVal.first, &(keyVal.second));
		i += T;
	}
}

void 
STLPQWorkerInsert(CoarseGrainPQ& q, vpii& keyVals, int id, int T)
{
	int i = id;
	while (i < N) {
		pii keyVal = keyVals[i];
		q.push(keyVal.first, keyVal.second);
		i += T;
	}
}

void 
PrintUsage()
{
	std::cout << "Usage: ./project-main <number-of-threads>\n";
}

// look into thread safe shared pointers (using atomic operations which take care of reference counting)
