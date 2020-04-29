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

#pragma once

#ifndef _SKIP_LIST_HPP_
#define _SKIP_LIST_HPP_

struct PQNode
{
	PQNode() {}

	PQNode(int height, int val) : val_(val), height_(height) {
		nxt_ = new struct PQNode *[height_];
		
		for (int i = 0; i < height_; ++i) {
			nxt_[i] = new PQNode; /* unsure how to get around this */
			nxt_[i]->val_ = val_;
			nxt_[i]->height_ = height_;
		}
	}

	int val_;
	int height_;
	struct PQNode** nxt_;
};


#endif /* _SKIP_LIST_HPP_ */

