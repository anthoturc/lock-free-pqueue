#include <climits>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cassert>
#include <random>


#include "lock-free-pqueue.h"

Node::Node(int val, int height) 
	: val_(val), height_(height) 
{
	/* 	would like to use a vector, but I do not know how
	 *	that would the locking strats in the paper
	 */
	nxt_ = new Node *[height_]; 
	for (int i = 0; i < height_; ++i) {
		nxt_[i] = nullptr;
	}

	/* these are not used in the serial SkipList implementation
	 * but they are made use of in the in the paper. 	
	 */
	prev_ = nullptr;
	validLvl_ = 0;
}

/**********************************************/
/*					   Serial Skip List`							*/
/**********************************************/
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
	if (size_ == 0) return false;
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
	if (size_ == 0) return false;
	return removeNode(val) != nullptr;
}

Node * 
SkipList::removeNode(int val)
{
	Node **lvlUpdates = new Node *[maxHeight_];
	Node *tmp = head_;
	int currHeight = maxHeight_ - 1;
	
	/* search for the node to delete, and in the process, collect
	 nodes that will be updated */
	while (currHeight >= 0) {
		while (tmp->nxt_[currHeight] && tmp->nxt_[currHeight]->val_ < val) {
			tmp = tmp->nxt_[currHeight];
		}
		lvlUpdates[currHeight] = tmp;
		--currHeight;
	}
	tmp = tmp->nxt_[0];
	if (!tmp || tmp->val_ != val) return nullptr;

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
	--size_;
	/* memory leak right here (should delete the node) */
	/* also, what if we have a bunch of empty top most levels :b (not going to delete) */
	return tmp;
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

/**********************************************/
/*					   Parallel Skip List`						*/
/**********************************************/

#define DEFAULT_MAX_LVL 20 /* corresponds to around 1,000,000 nodes */

PQueue::PQueue() : PQueue(DEFAULT_MAX_LVL) {}

PQueue::PQueue(int maxLevel) 
	: size_(0), maxLevel_(maxLevel) 
{
	int headVal = -1,
		tailVal = -1;
	head_ = createNode(maxLevel_, INT_MIN, &headVal);
	tail_ = createNode(maxLevel_, INT_MAX, &tailVal);

	for (int i = 0; i < maxLevel_; ++i) {
		head_->nxt_[i].node = tail_;
		head_->nxt_[i].w &= FALSE_MASK; /* lowest bit should be 0 */		
	}
}

bool
PQueue::push(int key, int *val)
{
	PQNode *node1, *node2, *newNode;
	PQNode **savedNodes = new PQNode * [maxLevel_];

	int level = randomLevel(); 
	newNode = createNode(level, key, val);
	copyNode(newNode);
	node1 = copyNode(head_);

	for (int i = maxLevel_ - 1; i >= 1; --i) {
		node2 = scanKey(&node1, i, key);
		releaseNode(node2);
		if (i < level) {
			savedNodes[i] = copyNode(node1);
		}
	}

	while (true) {
		node2 = scanKey(&node1, 0, key);
		PQVLink val2 = node2->val_;

		if (!isMarked(val2.w) && node2->key_ == key) {
			if (__sync_bool_compare_and_swap((uintptr_t *)&(node2->val_.w), node2->val_.w,  (uintptr_t)val)) {
				releaseNode(node1);
				releaseNode(node2);
				for (int i  = 1; i < level - 1; ++i) {
					releaseNode(savedNodes[i]);
				}
				releaseNode(newNode);
				releaseNode(newNode);
				return true;
			} else {
				releaseNode(node2);
				continue;
			}
		}

		newNode->nxt_[0].node = node2;
		newNode->nxt_[0].w &= FALSE_MASK;
		releaseNode(node2);
		PQLink newLink;
		newLink.node = newNode;
		newLink.w &= FALSE_MASK; // the lowest bit is a boolean 
		if (__sync_bool_compare_and_swap((uintptr_t *)(&(node1->nxt_[0].w)), (uintptr_t)(node1->nxt_[0].w), newLink.w)) {
			releaseNode(node1);
			break;
		}
	}

	for (int i = 1; i < level; ++i) {
		newNode->validLvl_ = i;
		node1 = savedNodes[i];

		while (true) {
			node2 = scanKey(&node1, i, key);
			newNode->nxt_[i].node = node2;
			newNode->nxt_[i].w &= FALSE_MASK;
			releaseNode(node2);
			PQLink newLink;
			newLink.node = newNode;
			newLink.w &= FALSE_MASK;
			if (isMarked(newNode->val_.w) ||
				__sync_bool_compare_and_swap((uintptr_t *)(&(node1->nxt_[i].w)), node1->nxt_[i].w, newLink.w)) {
				releaseNode(node1);
				break;
			}
		}
	}

	newNode->validLvl_ = level;
	if (isMarked(newNode->val_.w)) {
		newNode = helpDelete(newNode, 0);
	}

	releaseNode(newNode);
	size_++;
	return true;
}

PQNode *
PQueue::pop()
{
	// if (!size_.load()) return nullptr;

	PQNode *prev,
		*node1, 
		*node2;

	prev = copyNode(head_);
	while (true) {
		node1 = readNext(&prev, 0);
		if (node1 == tail_) {
			releaseNode(prev);
			releaseNode(node1);
			return nullptr;
		}
	retry: /* it was cool to see this forbidden jutsu being used */
		PQVLink valLink = node1->val_;
		int *val = (int *)getPointer(valLink.w);
		bool del = isMarked(valLink.w);

		if (node1 != (PQNode *)getPointer(prev->nxt_[0].w)) {
			releaseNode(node1);
			continue;
		}

		PQVLink newVLink;
		newVLink.p = val;
		newVLink.w |= 0x1; /* set lowest bit */
		if (!del) {
			if (__sync_bool_compare_and_swap((uintptr_t *)&(node1->val_.w), valLink.w, newVLink.w)) {
				node1->prev_ = prev;
				break;
			} else goto retry;
		} else if (del) { // why did they do this?
			node1 = helpDelete(node1, 0);
		}
		releaseNode(prev);
		prev = node1;
	}

	for (int i = 0; i < node1->lvl_; ++i) {
		PQLink v;
		PQLink toSet;
		bool del = true;
		while (true) {
			v = node1->nxt_[i];
			node2 = (PQNode *)getPointer(v.w);
			del = isMarked(v.w);
			
			toSet.node = node2;
			toSet.w |= 0x1; /* set lowest bit */			
			if (del || __sync_bool_compare_and_swap((uintptr_t *)(&(node1->nxt_[i].w)), node1->nxt_[i].w, toSet.w)) {
				break;
			}
		}
	}

	prev = copyNode(head_);
	for (int i = node1->lvl_ - 1; i >= 0; --i) {
		removeNode(node1, &prev, i);
	}

	PQVLink val = node1->val_;

	releaseNode(prev);
	releaseNode(node1);
	releaseNode(node1);

	size_--;
	return node1;
}

PQNode *
PQueue::readNext(PQNode **node1, int lvl)
{
	if (isMarked((*node1)->val_.w)) {
		*node1 = helpDelete(*node1, lvl);
	}

	PQNode *node2 = readNode((*node1)->nxt_[lvl]);
	while (!node2) {
		*node1 = helpDelete(*node1, lvl);
		node2 = readNode((*node1)->nxt_[lvl]);
	}

	return node2;
}

bool 
PQueue::isMarked(uintptr_t w)
{
	/* mask out the last two bits to get the boolean */
	return w & 0x1;
}

PQNode *
PQueue::scanKey(PQNode **node1, int lvl, int key)
{
	PQNode *node2 = readNext(node1, lvl);
	while (node2->key_ < key) {
		releaseNode(*node1);
		*node1 = node2;
		node2 = readNext(node1, lvl);
	}
	return node2;
}

PQNode *
PQueue::createNode(int lvl, int key, int *val)
{
	PQNode *node = mallocNode();
	node->prev_ = nullptr;
	node->validLvl_ = 0;
	node->lvl_ = lvl;
	node->key_ = key;
	node->nxt_ = new PQLink[lvl];
	node->val_.p =  val;
	node->val_.w &= FALSE_MASK; /* set lowest bit to false */
	return node;
}

void
PQueue::removeNode(PQNode *node, PQNode **prev, int level)
{
	PQNode *last;

	while (true) {
		PQLink l = node->nxt_[level];

		if ((PQNode *)getPointer(l.w) == nullptr && isMarked(l.w)) {
			break;
		}

		last = scanKey(prev, level, node->key_);
		releaseNode(last);

		if (last != node || 
			((PQNode *)getPointer(node->nxt_[level].w) == nullptr && !isMarked(node->nxt_[level].w))) {
			break;
		}

		PQLink newLink;
		newLink.node = (PQNode *)getPointer(node->nxt_[level].w);
		newLink.w &= FALSE_MASK; /* lowest bit to 0 */
		if (__sync_bool_compare_and_swap((uintptr_t *)&((*prev)->nxt_[level].w), (*prev)->nxt_[level].w, newLink.w)) {
			node->nxt_[level].node = nullptr;
			node->nxt_[level].w |= 0x1;
			break;
		}

		if ((PQNode *)getPointer(node->nxt_[level].w) == nullptr && isMarked(node->nxt_[level].w)) {
			break;
		}
	}
}

PQNode *
PQueue::mallocNode()
{
	/* I really do not know what they did here :/ */
	PQNode *newNode = new PQNode;
	assert(((uintptr_t)newNode & 0x1) == 0);
	return newNode;
}

PQNode *
PQueue::readNode(PQLink& addr)
{
	assert(addr.w != 0);
	/* node marked for deletion, so return a nullptr */
	if (isMarked(addr.w)) return nullptr;

	return (PQNode *)getPointer(addr.w);
}

PQNode *
PQueue::copyNode(PQNode *node)
{
	/* increase the reference count for this node */
	return node;
}

void 
PQueue::releaseNode(PQNode *node)
{
	/* recursively call releaseNode on the nodes that this
	node has owned pointers to (i.e. the prev pointer) */
}

void *
PQueue::getPointer(uintptr_t w)
{
	return (void *)(w & FALSE_MASK);
}

int 
PQueue::randomLevel()
{
  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(0,1); /* for heads or tails */
	/* simulate maxLevel_ coin tosses */
	int rndLevel = 1;
	for (int i = 1; i < maxLevel_; ++i) {
		if (distribution(generator) == 0) ++rndLevel;
	} 
	return rndLevel;
}

PQNode *
PQueue::helpDelete(PQNode *node, int lvl)
{
	PQNode *node2, *prev;
	for (int i = lvl; i < node->lvl_; ++i) {
		PQLink toSet;
		PQLink old;
		bool d;

		while (true) {
			d = isMarked(node->nxt_[i].w);
			node2 = (PQNode *)getPointer(node->nxt_[i].w);

			old.node = node2;
			old.w &= FALSE_MASK;
			
			toSet.node = node2;
			toSet.w |= 0x1; // set lowest bit to high 
			
			if (d || __sync_bool_compare_and_swap((uintptr_t *)(&(node->nxt_[i].w)), old.w, toSet.w)) {
				break;
			}
		}
	}

	prev = node->prev_;
	if (!prev || lvl >= prev->validLvl_) {
		prev = copyNode(head_);
		for (int i = maxLevel_ - 1; i >= lvl; --i) {
			node2 = scanKey(&prev, i, node->key_);
			releaseNode(node2);
		}
	} else {
		copyNode(prev);
	}

	removeNode(node, &prev, lvl);
	releaseNode(node);
	return prev;
}

int 
PQueue::size()
{
	return size_.load();
}

void
PQueue::debugPrint()
{
	PQNode *tmp = head_;

	while (tmp) {
		int *v = (int *)getPointer(tmp->val_.w);
		std::cout << tmp->key_ << "(" << *v << ") ";
		int lvl = tmp->lvl_;
		for (int i = 0; i < lvl; ++i) { // comment this loop out for just the list 
			std::cout << "- ";
		}
		std::cout << std::endl;
		tmp = (PQNode *)getPointer(tmp->nxt_[0].w);
	}
}