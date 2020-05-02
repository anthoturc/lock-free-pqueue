#include <climits>
#include <cstdlib>
#include <ctime>
#include <iostream>


#include "lock-free-pqueue.hpp"

Node::Node(int val, int height) : val_(val), height_(height) {
	/* 	would like to use a vector, but I do not know how
	 *	that would the locking strats in the paper
	 */
	nxt_ = new Node *[height_]; 
	for (int i = 0; i < height_; ++i) {
		nxt_[i] = nullptr;
	}
}


SkipList::SkipList() : size_(0), maxHeight_(1)
{
	/* random seed */
	srand(time(NULL));

	/* init the sentinnels of the list */
	head_ = new Node(
		INT_MIN, 							/* skip list is ordered so head should have value of -inf */
		maxHeight_
	);

	tail_ = new Node(
		INT_MAX,							/* '' so tial should have value of inf */
		maxHeight_
	);

	/* have the nxt pointers of the head column point to the tail */
	for (int i = 0; i < maxHeight_; ++i) {
		head_->nxt_[i] = tail_;
	}
}

/* return true if successfully inserted, false otherwise */
bool
SkipList::insert(int val)
{
	return insertNode(val) != nullptr;	
}

Node *
SkipList::insertNode(int val)
{

	if (contains(val)) return nullptr;
	/* choose a randome height for this node */
	int randHeight = chooseRandomHeight();

	/* this node coould be taller than the current height */
	if (randHeight > maxHeight_) resize(randHeight);

	/* 
	 *	Find the location of the node to insert by traversing the 
	 *	list, moving forward when the value of the next node is less then
	 *  the value to insert. Move down a level when the nxt ptr is null or,
	 *	when the next value becomes greater than the value to insert. 
	 *
	 *	We can think of this as moving down a stairs. Any node for which we go 
	 * 	down a level will need to have its corresponding nxt pointer updated so that 
	 *	it points to the corresponding new node at that level. These nodes are stored
	 *	in lvlUpdates
	 */
	int currMaxHeight = maxHeight_;
	Node ** lvlUpdates = new Node *[currMaxHeight];
	Node *tmp = head_;
	/* find the location to insert this node */
	int currHeight = maxHeight_ - 1;
	while (currHeight >= 0) {
		while (tmp->nxt_[currHeight] && tmp->nxt_[currHeight]->val_ < val) {
			tmp = tmp->nxt_[currHeight];
		}

		lvlUpdates[currHeight] = tmp;
		--currHeight;
	}

	Node *toAdd = new Node(val, randHeight);
	for (int i = 0; i < randHeight; ++i) {
		toAdd->nxt_[i] = lvlUpdates[i]->nxt_[i];
		lvlUpdates[i]->nxt_[i] = toAdd;
	}

	++size_;
	return toAdd;
}

bool
SkipList::contains(int val)
{
	/* check if the value returned */
	Node *nodeFound = findNode(val); 
	if (!nodeFound) return false;
	return nodeFound->nxt_[0]->val_ == val;
}

/*
 * 	contains will traverse the list by starting at the head and 
 * 	moving to the right when the `val' is greater than the next node at the current
 * 	the next level. 
 */
Node *
SkipList::findNode(int val)
{
	Node *tmp = head_;
	int currHeight = maxHeight_ - 1; /* start at the heighest level */
	while (currHeight >= 0) {

		/* if we we go off the list then we did not find the node */
		while (tmp->nxt_[currHeight] && tmp->nxt_[currHeight]->val_ < val) {
			tmp = tmp->nxt_[currHeight];
		}
		/* move down to the next level */
		--currHeight;
	}

	return tmp;
}


bool
SkipList::remove(int val)
{
	return removeNode(val);
}

bool 
removeNode(int val)
{
	Node **lvlUpdates = new Node *[maxHeight_];
	Node *tmp = head_;
	int currHeight = maxHeight_ - 1;
	
	/* search for the node to delete, and in the process, collect
	 nodes that will be updated */
	while (currHeight >= 0) {
		while (tmp->nxt_[currHeight] && tmp->nxt_[currHeight]->val_ < val) {
			tmp = tmp->nxt[currHeight];
		}
		lvlUpdates[currHeight] = tmp;
		--currHeight;
	}

	if (!tmp || !tmp->nxt_[0] || tmp->nxt_[0]->val_ != val) return false;

	for (int i = 0; i < maxHeight_; ++i) {
		/* if we reach a level whose nxt pointer is not tmp
		then we can be sure that the upper levels will not either
		this is because the nxt pointers of all previous levels must
		point to the same nxt pointer if THIS level will also point to the 
		nxt pointer*/
		if (lvlUpdates[i]->nxt_[i] != tmp) break;
		/* adjust the nxt pointers */
		lvlUpdates[i]->nxt_[i] = tmp->nxt_[i];
	}

	/* memory leak right here (should delete the node) */
	/* also, what if we have a bunch of empty top most levels :b (not going to delete) */
	return false;
}

void 
SkipList::resize(int lvl)
{ 
	/* add the new level to the head */
	Node *nxtHead = new Node(INT_MIN, lvl);
	for (int i = 0; i < maxHeight_; ++i) {
		nxtHead->nxt_[i] = head_->nxt_[i];
	}

	head_ = nxtHead;
	maxHeight_ = lvl;
}

int 
SkipList::chooseRandomHeight()
{
	/* not entirely sure how to go about making another random level */
	return (std::abs(rand()) % (maxHeight_ + size_)) + size_ + 1; 
}

void
SkipList::print()
{
	Node *tmp = head_;

	for (int i = 0; i < maxHeight_; ++i) {
		std::cout << "-  ";
	}
	std::cout << "\n";

	for (int i = 0; i < size_; ++i) {
		std::cout << tmp->nxt_[0]->val_ << " ";
		for (int cnt = tmp->nxt_[0]->height_; cnt > 0; --cnt) {
			std::cout << "-  ";
		}
		std::cout << "\n";
		tmp = tmp->nxt_[0];
	}
}
