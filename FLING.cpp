#include "FLING.h"
#include "MurmurHash3.h"
#include <algorithm>
#include <bitset>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <math.h>
#include <set>
#include <sstream>
#include <string.h>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

vector<uint> FLING::get_hashed_row_indices(uint index) {
  string key = to_string(index);
  uint key_length = to_string(index).size();
  vector<uint> *hashvals = new vector<uint>;
  uint op;
  for (uint i = 0; i < row_count; i++) {
    MurmurHash3_x86_32(key.c_str(), key_length, i, &op); // seed is row number
    hashvals->push_back(op % blooms_per_row);
  }
  return *hashvals;
}

FLING::FLING(uint row_count, uint blooms_per_row, LSH* hash_function, uint hash_bits, 
  uint hash_repeats, uint num_points) {

  this->row_count = row_count;
  this->blooms_per_row = blooms_per_row;
  this->num_bins = blooms_per_row * row_count;
  this->hash_repeats = hash_repeats;
  this->num_points = num_points;
  this->hash_function = hash_function;
  this->hash_size = hash_size;
  this->internal_hash_bits = hash_bits;
  this->internal_hash_length = 1 << internal_hash_bits;
  this->points_added_so_far = 0;
  this->rambo_array = new vector<uint>[hash_repeats * internal_hash_length];


  // Create meta rambo
  meta_rambo = new vector<uint>[row_count * blooms_per_row];
  for (uint point = 0; point < num_points; point++) {
    vector<uint> hashvals = FLING::get_hashed_row_indices(point);
    for (uint r = 0; r < row_count; r++) {
      meta_rambo[hashvals[r] + blooms_per_row * r].push_back(point);
    }
  }

  // Sort array entries in meta rambo
  for (uint i = 0; i < num_bins; i++) {
    sort(meta_rambo[i].begin(), meta_rambo[i].end());
  }
}

FLING::~FLING() {
  delete[] this->rambo_array;
  delete[] this->meta_rambo;
}

/**
 * Inserts a keys of a given index into the FLING array
 */
void FLING::insert(int num_inputs, int* data_ids, float* data_vals, int* data_marker) {
  unsigned int* hashes = new unsigned int[hash_repeats * num_inputs];
	unsigned int* indices = new unsigned int[hash_repeats * num_inputs];

	hash_function->getHash(hashes, indices, data_ids, data_vals, data_marker, num_inputs, 1);

  vector<uint> row_indices_arr[num_inputs];
  for (uint i = 0; i < num_inputs; i++){
    row_indices_arr[i] = get_hashed_row_indices(i + points_added_so_far);
  }

  for (uint rep = 0; rep < hash_repeats; rep++) {
    for (uint index = 0; index < num_inputs; index++) {
      vector<uint> row_indices = row_indices_arr[index];
      for (uint r = 0; r < row_count; r++) {
        uint b = row_indices.at(r);
        rambo_array[rep * internal_hash_length + hashes[rep * num_inputs + index]]
            .push_back(r * blooms_per_row + b);
      }
    }
  }

  points_added_so_far += num_inputs;
}

/**
 * Finishes FLING construction by sorting all buckets for fast access. All
 * points must be inserted at this point.
 */
void FLING::finalize_construction() {

  // Remove duplicates
  for (uint i = 0; i < internal_hash_length * hash_repeats; i++) {
    sort(rambo_array[i].begin(), rambo_array[i].end());
    rambo_array[i].erase(unique(rambo_array[i].begin(), rambo_array[i].end()),
                         rambo_array[i].end());
  }
}

void FLING::query(int* data_ids, float* data_vals, int* data_marker, uint query_goal, uint *query_output) {
	uint* hashes = new unsigned int[hash_repeats];
	uint* indices = new unsigned int[hash_repeats]; // Should be all one value

  hash_function->getHash(hashes, indices, data_ids, data_vals, data_marker, 1, 1);

  // Get observations, ~35%
  vector<uint> counts(num_bins, 0);
  for (uint rep = 0; rep < hash_repeats; ++rep) {
    for (uint rambo_cell : rambo_array[internal_hash_length * rep + hashes[rep]]) {
      ++counts[rambo_cell];
    }
  }

  // Populate sorted, ~25%
  vector<uint> sorted[hash_repeats + 1];
  uint size_guess = num_bins / (hash_repeats + 1);
  for (vector<uint> &v : sorted) {
    v.reserve(size_guess);
  }
  for (uint i = 0; i < num_bins; ++i) {
      sorted[counts[i]].push_back(i);
  }

  // Determine the earliest goal_num_points that occur R times, ~30%
  vector<uint8_t> num_counts(num_points, 0); 
  uint num_found = 0;
  for (uint rep = hash_repeats; rep > 0; --rep) {
    for (uint bin : sorted[rep]) {
      for (uint point : meta_rambo[bin]) {
         if (++num_counts[point] == row_count) {
          query_output[num_found] = point;
          if (++num_found == query_goal) {
            return;
          }
        }
      }
    }
  }

  // Only get down here if not adding zeros, depending on the test
  for (uint i = 0; num_found < query_goal; i++) {
    if (num_counts[i] != row_count) {
      query_output[num_found++] = i;
    }
  }
}