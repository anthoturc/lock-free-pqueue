/*
 * lock-free-pqueue.hpp
 *
 * 	Class definitions of lock-free priority queue. This implementation is based 
 *	on a design found in:
 * 
 *	Fast and Lock-Free Concurrent Priority Queues for Multi-Thred Systems
 *	(http://www.non-blocking.com/download/SunT03_PQueue_TR.pdf)
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

#ifndef _LOCK_FREE_PQUEUE_HPP_
#define _LOCK_FREE_PQUEUE_HPP_

/* forward declarations */
class Node;
class SkipList;

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
		validheight_;
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

/*
 * 	This implementation of the skip list is based on the 
 *	implementation presented in the paper  
 */
class PQueue
{
public:
	PQueue();

	bool push(int val); /* equivalent to the insert method in the paper */
	Node* pop(); /* equivalent to the deletemin method in the paper */

private:
	/* used to traverse nodes in the skiplist */
	Node *readNext(Node **node1, int lvl);
	Node *scanKey(Node *node1, int lvl, int val);
	Node *helpDelete(Node *node1, int lvl);

	int size_;
};


#endif /* _LOCK_FREE_PQUEUE_HPP_ */