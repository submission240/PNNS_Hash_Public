#pragma once

#include <algorithm>
#include <cassert>
#include <vector>

#include "def.h"
#include "util.h"
#include "pri_queue.h"
#include "qalsh.h"

// -----------------------------------------------------------------------------
//  NH_Hash: Nearest Hyperplane Hash
// -----------------------------------------------------------------------------
class NH_Hash {
public:
    NH_Hash(						// constructor
		int   n,						// number of data objects
		int   d,						// dimension of data objects
		int   m,						// #hash tables
		const float *data);				// input data

	// -------------------------------------------------------------------------
    ~NH_Hash();						// destructor

	// -------------------------------------------------------------------------
	void display();					// display parameters

	// -------------------------------------------------------------------------
	int nns(						// point-to-hyperplane NNS
		int   top_k,					// top-k value
		int   l,						// collision threshold
		int   cand,						// #candidates
		const float *query,				// input query
		MinK_List *list);				// top-k results (return)

	// -------------------------------------------------------------------------
	int64_t get_memory_usage()		// get memory usage
	{
		int64_t ret = 0;
		ret += sizeof(*this);
		ret += lsh_->get_memory_usage();
		return ret;
	}

protected:
    int   n_pts_;					// number of data objects
	int   dim_;						// dimension of data objects
	int   nh_dim_;					// new data dimension after transformation
	float M_;						// max l2-norm of o' after transformation
	
	const float *data_;				// original data objects
	QALSH *lsh_;					// QALSH for nh data

	// -------------------------------------------------------------------------
	void transform_data(			// data transformation
		const float *data,				// input data
		float &norm,					// l2-norm-sqr of nh_data (return)
		float *nh_data);				// nh_data (return)

	// -------------------------------------------------------------------------
	void transform_query(			// query transformation
		const float *query,				// input query
		float *nh_query);				// nh_query after transform (return)
};
