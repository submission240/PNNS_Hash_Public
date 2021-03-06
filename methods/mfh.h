#pragma once

#include <algorithm>
#include <cassert>
#include <vector>

#include "def.h"
#include "util.h"
#include "pri_queue.h"
#include "rqalsh.h"

// -----------------------------------------------------------------------------
//  MFH_Hash: Multi-Partition Furthest Hyperplane Hash 
// -----------------------------------------------------------------------------
class MFH_Hash {
public:
	MFH_Hash(						// constructor
		int   n,						// number of data objects
		int   d,						// dimension of data objects
		int   m,						// #hash tables
		float b,						// interval ratio
		const float *data);				// input data

	// -------------------------------------------------------------------------
    ~MFH_Hash();					// destructor

	// -------------------------------------------------------------------------
	void display();					// display parameters

	// -------------------------------------------------------------------------
	int nns(						// point-to-hyperplane NNS
		int   top_k,					// top-k value
		int   l,						// separation threshold
		int   cand,						// #candidates
		const float *query,				// input query
		MinK_List *list);				// top-k results (return)

	// -------------------------------------------------------------------------
	int64_t get_memory_usage()		// get memory usage
	{
		int64_t ret = 0;
		ret += sizeof(*this);
		ret += SIZEFLOAT * fh_dim_; // centroid_
		ret += SIZEINT * n_pts_;	// shift_id_
		for (auto hash : hash_) { 	// blocks_
			ret += hash->get_memory_usage();
		}
		return ret;
	}

protected:
    int   n_pts_;					// number of data objects
	int   dim_;						// dimension of data objects
	int   fh_dim_;					// new data dimension after transformation
	float b_;						// interval ratio
	float M_;						// max l2-norm sqr of o'
	const float *data_;				// original data objects

	int *shift_id_;					// shift data id
	int block_cand_ = 10000;		// max #candidates checked in each block
	std::vector<RQALSH*> hash_;		// blocks

	// -------------------------------------------------------------------------
	void calc_transform_centroid(	// calc centorid after data transformation
		const float *data,				// input data
		float &norm,					// norm of fh_data (return)
		float *centroid);				// centroid (return)

	// -------------------------------------------------------------------------
	float calc_transform_dist(		// calc l2-dist after data transformation 
		const float *data,				// input data
		const float *centroid);			// centroid after data transformation

	// -------------------------------------------------------------------------
	void transform_data(			// data transformation
		const float *data,				// input data
		float *fh_data);				// fh data (return)

	// -------------------------------------------------------------------------
	void transform_query(			// query transformation
		const float *query,				// input query
		float &norm_q,					// l2-norm sqr of q after transform (return)
		float *fh_query);				// fh_query after transform (return)
};
