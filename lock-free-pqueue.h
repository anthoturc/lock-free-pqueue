/*
 * lock-free-pqueue.hpp
 *
 * 	Class definitions of lock-free priority queue. This implementation is based 
 *	on a design found in:
 * 
 *	Fast and Lock-Free Concurrent Priority Queues for Multi-Thred Systems
 *	(http://www.non-blocking.com/download/SunT03_PQueue_TR.pdf)
 *	(http://www.cse.chalmers.se/~tsigas/papers/JPDC-Lock-free-skip-lists-and-Queues.pdf)
 * 	
 * 	by Hakan Sundell and Philippas Tsigas. 
 *
 *	The Priority Queue ADT specifies that keys are used as the priorities for
 *	values. This implementation assumes that the keys and values are signed integers. This implies 
 *  that the values can (and will) serve as the keys.
 *
 *	If there is time, the lock-free-priority queue will be templeted to allow for the storage
 * 	of records.
 */

#ifndef _LOCK_FREE_PQUEUE_H_
#define _LOCK_FREE_PQUEUE_H_

#include <cstdint>
#include <stdint.h>
#include <atomic>

/* forward declarations */
class Node;
class SkipList;
struct PQNode;
class PQueue;

/* 	Each Node will maintain a:
 *		value, 
 * 		height, 
 * 		and a list of Nodes that follow it
 */
class Node
{
public:
	Node(int height, int val);
	
	/* these members are based on those foudnd in the paper listed above */
	int val_, 
		height_, 
		validLvl_;
	Node** nxt_, * prev_;
};

/*
 * 	My implementation of the skip list to be used by non parallel programs
 *
 *	I wrote this to help with my understanding of the data structure. 
 */
class SkipList
{
public:
	SkipList();

	bool insert(int val);
	bool contains(int val);
	bool remove(int val);

	void print(); /* used for debugging */


private:
	int size_, maxHeight_;
	Node *head_, *tail_;

	Node* findNode(int val);
	Node* insertNode(int val);
	Node* removeNode(int val);
	void resize(int lvl);
	int chooseRandomHeight();

};

/**********************************************/
/*					   Parallel Skip List`						*/
/**********************************************/

/* the remainder of this file is based on the paper (linked above) */

const uintptr_t FALSE_MASK = (uint64_t)(-1) << 1; /* all but last bit are 1 */


/*
 *	allows for using the last bit of the pointer
 *	as a true/false value (see paper)
 */
union PQLink
{
	uintptr_t w; /* paper uses this to when a node should be deleted */
	PQNode *node;
};

union PQVLink
{
	uintptr_t w; /* paper uses this to determine when to delete */
	int *p;
};

/*
 *	The `key_' field of the PQueue Node will
 *	simply hold keys. With enough time I may be able to implement
 *	a version of the PQNode that also holds an arbitrary value
 */
struct PQNode
{
	int lvl_, 
		key_, 
		validLvl_;
	PQLink *nxt_;
	PQVLink val_;
	PQNode *prev_;
};

/*
 * 	This implementation of the skip list is based on the 
 *	implementation presented in the paper  
 */
class PQueue
{
public:
	PQueue(int maxLevel_);
	PQueue();
	bool push(int key, int* val); /* equivalent to the insert method in the paper */
	PQNode *pop(); /* equivalent to the deletemin method in the paper */

	void debugPrint();

private:
	PQNode *createNode(int lvl, int key, int *val);

	/* used to traverse nodes in the skiplist */
	PQNode *readNext(PQNode **node1, int lvl);
	bool isMarked(uintptr_t w);

	PQNode *scanKey(PQNode **node1, int lvl, int key);
	PQNode *helpDelete(PQNode *node1, int lvl);

	void removeNode(PQNode *node, PQNode **prev, int level);

	/* safe memory management of nodes (based on what was in paper) 
		still unsure about the details of these */
	PQNode *mallocNode();
	PQNode *readNode(PQLink& addr);
	PQNode *copyNode(PQNode *node);
	void releaseNode(PQNode *node);

	void *getPointer(uintptr_t w);


	int randomLevel();

	int maxLevel_;

	PQNode *head_, *tail_;
};

#endif /* _LOCK_FREE_PQUEUE_H_ */