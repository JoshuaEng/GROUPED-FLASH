
#ifndef _FLING_
#define _FLING_
#include "LSH.h"
#include <bitset>
#include <set>
#include <string>
#include <vector>

class FLING {

private:
  std::vector<uint> *get_hashed_row_indices(uint index);

  uint row_count;
  uint blooms_per_row;
  uint num_bins;
  uint hash_repeats;
  uint num_points;
  uint internal_hash_length;
  uint internal_hash_bits;
  uint hash_size;
  std::vector<uint> *rambo_array;
  std::vector<uint> *meta_rambo;
  uint *hashes;
  uint num_hashes_generated;
  uint *sorted;
  LSH* hash_function;

public:
  FLING(uint row_count, uint blooms_per_row, uint *hashes, uint num_hashes_generated, LSH* hash_function, uint hash_bits,
        uint hash_repeats, uint num_points);
  ~FLING();

  void do_inserts();
  void query(uint* query_hashes, uint i, uint TOPK, uint32_t* recall_buffer);
  void finalize_construction();
  uint get_hash_index(uint i);
};

#endif
