
#ifndef _FLING_
#define _FLING_
#include <bitset>
#include <set>
#include <string>
#include <vector>
#include "LSH.h"

class FLING {

private:
  std::vector<uint> get_hashed_row_indices(uint index);

  uint row_count;
  uint blooms_per_row;
  uint num_bins;
  uint hash_repeats;
  uint num_points;
  uint internal_hash_length;
  uint internal_hash_bits;
  uint hash_size;
  uint points_added_so_far;
  std::vector<uint> *rambo_array; 
  std::vector<uint> *meta_rambo;
  LSH* hash_function;
  uint *sorted;

public:
  FLING(uint row_count, uint blooms_per_row, LSH* hash_function, uint hash_bits, 
  uint hash_repeats, uint num_points);
  ~FLING();

  void insert(int num_inputs, int* data_ids, float* data_vals, int* data_marker);
  void query(int* data_ids, float* data_vals, int* data_marker, uint query_goal, uint *query_output);
  void finalize_construction();
  uint get_hash_index(uint i);
};

#endif
