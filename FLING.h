
#ifndef _FLING_
#define _FLING_
#include <bitset>
#include <set>
#include <string>
#include <vector>
#include "LSH.h"

class FLING {

private:
  std::vector<uint> get_hashed_row_indices(size_t index);

  size_t row_count;
  size_t blooms_per_row;
  size_t num_bins;
  size_t hash_repeats;
  size_t num_points;
  size_t internal_hash_length;
  size_t internal_hash_bits;
  size_t hash_size;
  std::vector<uint32_t> *rambo_array; 
  std::vector<int> *meta_rambo;
  uint32_t *records;
  LSH* hash_function;

public:
  FLING(size_t row_count, size_t blooms_per_row, LSH* hash_function, size_t hash_bits, 
  size_t hash_repeats, size_t num_points);
  ~FLING();

  void insert(int numInputEntries, int* dataIdx, float* dataVal, int* dataMarker);
  std::vector<int> *query(const std::vector<double> &vec, size_t goal_num_points);
  void finalize_construction();
  size_t get_hash_index(size_t i);
};

#endif
