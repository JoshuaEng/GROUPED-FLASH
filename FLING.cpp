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

vector<uint> FLING::get_hashed_row_indices(size_t index) {
  string key = to_string(index);
  size_t key_length = to_string(index).size();
  vector<uint> *hashvals = new vector<uint>;
  uint op;
  for (size_t i = 0; i < row_count; i++) {
    MurmurHash3_x86_32(key.c_str(), key_length, i, &op); // seed is row number
    hashvals->push_back(op % blooms_per_row);
  }
  return *hashvals;
}

FLING::FLING(size_t row_count, size_t blooms_per_row, LSH* hash_function, size_t hash_bits, 
  size_t hash_repeats, size_t num_points) {

  this->row_count = row_count;
  this->blooms_per_row = blooms_per_row;
  this->num_bins = blooms_per_row * row_count;
  this->hash_repeats = hash_repeats;
  this->num_points = num_points;
  this->hash_function = hash_function;
  this->hash_size = hash_size;
  this->internal_hash_bits = hash_bits;
  this->internal_hash_length = 1 << internal_hash_bits;
  this->rambo_array = new vector<uint32_t>[hash_repeats * internal_hash_length];
  this->records = new uint32_t[hash_repeats * num_bins];


  // Create meta rambo
  meta_rambo = new vector<int>[row_count * blooms_per_row];
  for (size_t point = 0; point < num_points; point++) {
    vector<uint> hashvals = FLING::get_hashed_row_indices(
        point); // R hashvals, each with max value B
    for (size_t r = 0; r < row_count; r++) {
      meta_rambo[hashvals[r] + blooms_per_row * r].push_back(point);
    }
  }

  // Sort array entries in meta rambo
  for (size_t i = 0; i < num_bins; i++) {
    vector<int> to_sort = meta_rambo[i];
    sort(to_sort.begin(), to_sort.end());
  }
}

FLING::~FLING() {
  delete this->hash_function;
  delete[] this->rambo_array;
  delete[] this->meta_rambo;
}

/**
 * Inserts a keys of a given index into the FLING array
 */
void FLING::insert(int num_inputs, int* data_ids, float* data_vals, int* data_marker) {
  unsigned int* hashes = new unsigned int[hash_repeats * num_inputs];
	unsigned int* indexes = new unsigned int[hash_repeats * num_inputs];

	hash_function->getHash(hashes, indexes, data_ids, data_vals, data_marker, num_inputs, 1);

  for (int i = 0; i < hash_repeats * num_inputs; i++) {
    cout << indexes[i] << " " << hashes[i] << endl;
  }

  // vector<uint> row_indices = FLING::get_hashed_row_indices(index); // R hashvals

  // for (uint32_t r = 0; r < row_count; r++) {
  //   uint32_t b = row_indices.at(r);
  //   for (uint32_t rep = 0; rep < hash_repeats; rep++) {
  //     rambo_array[rep * internal_hash_length + bloom_positions[rep]]
  //         .push_back(r * blooms_per_row + b);
  //   }
  // }
}

/**
 * Finishes FLING construction by sorting all buckets for fast access. All
 * points must be inserted at this point.
 */
void FLING::finalize_construction() {

  // Remove duplicates
  for (size_t i = 0; i < internal_hash_length * hash_repeats; i++) {
    sort(rambo_array[i].begin(), rambo_array[i].end());
    rambo_array[i].erase(unique(rambo_array[i].begin(), rambo_array[i].end()),
                         rambo_array[i].end());
  }
}

vector<int> *FLING::query(const std::vector<double> &vec, size_t goal_num_points) {
  uint64_t bloom_positions[hash_repeats];
  // get_bloom_positions(bloom_positions, vec);

  // Get observations
  size_t index = 0;
  for (size_t rep = 0; rep < hash_repeats; rep++) {
    vector<uint32_t> to_add = rambo_array[internal_hash_length * rep + bloom_positions[rep]];
    for (uint32_t rambo_cell : to_add) {
      records[index++] = rambo_cell;
    }
  }

  // Populate sorted counts array in descending order
  vector<size_t> counts(row_count * blooms_per_row);
  for (size_t i = 0; i < index; i++) {
    counts[records[i]]++;
  }
  vector<size_t> count_counts(hash_repeats + 1, 0);
  for (size_t count : counts) {
    count_counts[count]++;
  }
  vector<size_t> offsets(hash_repeats + 1, 0);
  for (size_t i = 1; i <= hash_repeats; i++) {
    offsets[i] = count_counts[i - 1] + offsets[i - 1];
  }
  vector<size_t> sorted(row_count * blooms_per_row);
  for (size_t i = 0; i < row_count * blooms_per_row; i++) {
    size_t this_count = counts[i];
    sorted[num_bins - offsets[this_count] - 1] = i;
    offsets[this_count]++;
  }

  // Determine the earliest goal_num_points that occur R times
  vector<uint8_t> num_counts(num_points, 0); 
  int threshhold = 0;
  vector<int> *points = new vector<int>();
  size_t num_found = 0;
  // These ones we know can't exceed count
  for (size_t i = 0; i < row_count - 1; i++) {
    for (int point : meta_rambo[sorted[i]]) {
      num_counts[point]++;
    }
  }
  // Determine the first goal_num_points that exceed count
  for (size_t i = row_count - 1; ; i++) {
    for (int point : meta_rambo[sorted[i]]) {
      if (++num_counts[point] == row_count) {
        points->push_back(point);
        if (++num_found == goal_num_points) {
          goto done;
        }
      }
    }
  }
  done:;

  cout << "Using threshhold " << threshhold << "\n";

  return points;
}
