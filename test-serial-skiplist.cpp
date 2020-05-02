#include "lock-free-pqueue.hpp"

#include <iostream>
#include <cstdlib>
#include <vector>


#define MAX_RANGE 1001

bool containsAllVals(std::vector<int>&, SkipList&);
bool shouldNotContain(std::vector<int>&, SkipList&);

int
main()
{

	SkipList l;
	std::vector<int> vals(1001, 0);
	for (int i = 1; i < 50; ++i) {
		int idx = (rand() % MAX_RANGE);
		vals[idx] = 1;
		l.insert(idx);
	}

	if (!containsAllVals(vals, l)) {
		std::cout << "some values are missing\n";
	}

	if (!shouldNotContain(vals, l)) {
		std::cout << "some values should not be in the list\n";
	}

	return 0;
}


bool 
containsAllVals(std::vector<int>& vals, SkipList& l)
{
	for (int i = 0; i < MAX_RANGE; ++i) {
		if (vals[i])
			if (!l.contains(i)) return false;
	}

	return true;
}

bool
shouldNotContain(std::vector<int>& vals, SkipList& l)
{
	for (int i = 0; i < MAX_RANGE; ++i) {
		if (!vals[i])
			if (l.contains(i)) return false;
	}

	return true;
}