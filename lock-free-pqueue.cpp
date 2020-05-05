#include <climits>
#include <cstdlib>
#include <ctime>
#include <iostream>


#include "lock-free-pqueue.hpp"

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

#define DEFAULT_MAX_LVL 10 /* corresponds to around 2^10 levels */

PQueue::PQueue() : PQueue(DEFAULT_MAX_LVL) {}

PQueue::PQueue(int maxLevel) : size_(0), maxLevel_(maxLevel) {}

bool
PQueue::push(int key, int *val)
{
	Node *node1, *node2, *newNode;
	Node **savedNodes = new Node *[maxLevel_];

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
		node2 = scanKey(&node1, 0, val);
		PQVLink val2 = node2->val_;

		if (!isMarked(val2) && node2->key_ != key) {
			if (__sync_bool_compare_and_swap(&(node2->val_), val2, val)) {
				releaseNode(node1);
				releaseNode(node2);
				for (int i  = 1; i < level - 1; ++i) {
					releaseNode(savedNodes[i])
				}
				releaseNode(newNode);
				releaseNode(newNode);
				return true;
			} else {
				releaseNode(node2);
				continue;
			}
		}

		newNode->nxt_[0]->ptr32 = { node2, false };
		releaseNode(node2);
		if (__sync_bool_compare_and_swap(&(node1->nxt_[0]), { node2, false }, { newNode, false })) {
			releaseNode(node1);
			break;
		}
	}

	for (int i = 0; i < randLvl; ++i) {
		newNode->validLvl_ = i;
		node1 = savedNodes[i];

		while (true) {
			node2 = scanKey(&node1, i, val);
			newNode->nxt_[i]->ptr32 = { node2, false };
			releaseNode(node2);
			if (isMarked(newNode->val_) ||
				__sync_bool_compare_and_swap(&(node1->nxt_[i]), node1->nxt_[i], { newNode, false })) {
				releaseNode(node1);
				break;
			}
		}
	}

	newNode->validLvl_ = level;
	if (isMarked(newNode->val_)) {
		newNode = helpDelete(newNode, 0);
	}

	releaseNode(newNode);
	return true;
}

int *
PQueue::pop()
{
	Node *prev, 
		*last, 
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
		int *val = valLink.ptr32.p;
		bool del = valLink.ptr32.del;

		if (node1 != prev->nxt_[0]->ptr32.node) {
			releaseNode(node1);
			continue;
		}

		if (!del) {
			if (__sync_bool_compare_and_swap(&(node1->val_), valLink, { val, true })) {
				node1->prev_ = prev;
				break;
			} else goto retry;
		} else if (del) { // why did they do this?
			node1 = helpDelete(node1, 0);
		}
		releaseNode(prev);
		prev = node1;
	}

	for (int i = 0; i < node1->lvl_ - 1; ++i) {
		do {
			PQLink& v = node1->nxt_[i];
		} while (v.ptr32.del || __sync_bool_compare_and_swap(&(node1->nxt_[i]), { node2, false }, { node2, true }));
	}
	prev = copyNode(head);
	for (int i = node1->lvl_ - 1; i >= 0; --i) {
		removeNode(node1, &prev, i);
	}

	PQVLink val = node1->val_;

	releaseNode(prev);
	releaseNode(node1);
	releaseNode(node1);

	return val.ptr32.p;
}


Node *
PQueue::readNext(Node **node1, int lvl)
{
	if (isMarked((*node1)->val_)) {
		*node1 = helpDelete(*node1, lvl);
	}

	Node *node2 = readNode((*node1)->nxt_[lvl]);
	while (!node2) {
		*node1 = helpDelete(*node1, lvl);
		node2 = readNode((*node1)->nxt_[lvl]);
	}

	return node2;
}

bool 
PQueue::isMarked(PQVLink& val)
{
	return val.ptr32.del;
}

Node *
PQueue::scanKey(Node **node1, int lvl, int key)
{
	Node *node2 = readNext(node1, lvl);
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
	node->val_.ptr32 = { val, false };
	return node;
}

PQNode *
PQueue::mallocNode()
{
	/* I really do not know what they did here :/ */
	return new PQNode;
}

PQNode *
PQueue::readNode(PQLink& addr)
{
	/* node marked for deletion, so return a nullptr */
	if (addr.ptr32.del) return nullptr;

	return addr.ptr32.node;
}

PQNode *
PQueue::copyNode(PQNode *node)
{

}

void
PQueue::removeNode(Node *node, Node **prev, int level)
{
	Node *last;

	while (true) {
		PQLink l = node->nxt_[level];

		if (l.ptr32.node == nullptr && l.ptr32.del) {
			break;
		}

		last = scanKey(prev, level, node->key_);
		releaseNode(last);

		PQVLink v; = { nullptr, true };
		if (last != node || )
	}
}

void releaseNode(PQNode *node);