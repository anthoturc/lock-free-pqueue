#include "lock-free-pqueue.hpp"

#include <iostream>
#include <cstdlib>
#include <vector>


#define MAX_RANGE 1001

bool containsAllVals(std::vector<int>&, SkipList&);
bool shouldNotContain(std::vector<int>&, SkipList&);
bool checkUnique();
bool checkInsertAndLookup();


int
main()
{
	if (checkInsertAndLookup()) {
		std::cout << "\033[1;32mPASSED\033[0m: " << "checkInsertAndLookup\n"; 
	} else {
		std::cout << "\033[1;31mFAILED\033[0m: " << "checkInsertAndLookup\n"; 
	}

	std::cout << "-----------\n";
	if (checkUnique()) {
		std::cout << "\033[1;32mPASSED\033[0m: " << "checkUnique\n"; 
	} else {
		std::cout << "\033[1;31mFAILED\033[0m: " << "checkUnique\n"; 
	}
	checkUnique();

	return 0;
}

bool
checkUnique()
{
	SkipList l;
	bool success = l.insert(10);
	if (!success)
		return false;

	success = l.insert(10);
	if (success)
		return false;

	return true;
}

bool
checkInsertAndLookup()
{
	SkipList l;
	std::vector<int> vals(1001, 0);
	for (int i = 1; i < 15; ++i) {
		int idx = (rand() % MAX_RANGE);
		vals[idx] = 1;
		l.insert(idx);
	}

	if (!containsAllVals(vals, l)) {
		std::cout << "some values are missing\n";
		return true;
	}

	if (!shouldNotContain(vals, l)) {
		std::cout << "some values should not be in the list\n";
		return false;
	}

	return true;
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