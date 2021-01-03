#include <cmath>

#include "FLING.h"
#include "LSHReservoirSampler.h"
#include "MatMul.h"
#include "benchmarking.h"
#include "dataset.h"
#include "evaluate.h"
#include "indexing.h"
#include "misc.h"
#include "omp.h"

#include "FrequentItems.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void do_group(size_t B, size_t R, size_t REPS, size_t range, uint *hashes,
              uint max_reps, unsigned int *gtruth_indice, float *gtruth_dist,
              int *query_sparse_indice, float *query_sparse_val, int *query_sparse_marker, 
              LSH *hash_family) {

  unsigned int *queryOutputs = new unsigned int[NUMQUERY * TOPK]();

  auto begin = Clock::now();
  auto end = Clock::now();
  float etime_0;

  // Create index
  FLING *fling = new FLING(R, B, hashes, max_reps, hash_family, range, REPS, NUMBASE);
  fling->finalize_construction();

  // Do queries
  cout << "Querying..." << endl;
  omp_set_num_threads(1);
  begin = Clock::now();
  for (uint i = 0; i < NUMQUERY; i++) {
    uint32_t *recall_buffer = new uint32_t[TOPK];
#ifdef YFCC
    fling->query(query_sparse_indice, query_sparse_val + DIMENSION * i, query_sparse_marker, TOPK, recall_buffer);
#else
    fling->query(query_sparse_indice, query_sparse_val, query_sparse_marker + i, TOPK, recall_buffer);
#endif
    for (size_t j = 0; j < TOPK; j++) {
      queryOutputs[TOPK * i + j] = recall_buffer[j];
    }
    delete[] recall_buffer;
  }
  end = Clock::now();
  omp_set_num_threads(20);

  etime_0 = (end - begin).count() / 1000000;
  std::cout << "Queried " << NUMQUERY << " datapoints, used " << etime_0
            << "ms. \n";

  evaluate(queryOutputs, NUMQUERY, TOPK, gtruth_indice, gtruth_dist, AVAILABLE_TOPK);

  delete fling;
  delete[] queryOutputs;
}


void do_normal(size_t RESERVOIR, size_t REPS, size_t range, uint *hashes, uint *indices, 
              uint max_reps, unsigned int *gtruth_indice, float *gtruth_dist,
              int *query_sparse_indice, float *query_sparse_val, int *query_sparse_marker, 
              LSH *hashFamily) {

  unsigned int *queryOutputs = new unsigned int[NUMQUERY * TOPK]();

  auto begin = Clock::now();
  auto end = Clock::now();
  float etime_0;

  // Dimension not used so just pass in -1
  // Initialize hashtables and other datastructures.
  LSHReservoirSampler *myReservoir = new LSHReservoirSampler(
      hashFamily, RANGE, REPS, RESERVOIR, -1, range, NUMBASE, 1, 1, 1); 


  for (size_t start = 0; start < NUMBASE;) {
    size_t end = min(start + NUMBASE / NUMHASHBATCH, (size_t)NUMBASE);
    auto indexFunc = [start](size_t table, size_t probe) { 
      return (table * (size_t) (NUMBASE) + start + probe); 
    };
    myReservoir->add(end - start, hashes, indices, indexFunc);
    start = end;
  }

  std::cout << "Querying..." << endl;
  omp_set_num_threads(1);
  begin = Clock::now();
  myReservoir->ann(NUMQUERY, query_sparse_indice, query_sparse_val, query_sparse_marker,
                   queryOutputs, TOPK);
  end = Clock::now();
  omp_set_num_threads(20);
  etime_0 = (end - begin).count() / 1000000;
  std::cout << "Queried " << NUMQUERY << " datapoints, used " << etime_0
            << "ms." << endl;
 
  evaluate(queryOutputs, NUMQUERY, TOPK, gtruth_indice, gtruth_dist, AVAILABLE_TOPK);

  delete myReservoir;
  delete[] queryOutputs;
}

void benchmark_sparse() {

  omp_set_num_threads(20);

  float etime_0, etime_1, etime_2;
  auto begin = Clock::now();
  auto end = Clock::now();

  std::cout << "Reading groundtruth and data ... " << std::endl;
  begin = Clock::now();

  // Read in data and queries
#ifdef YFCC
  int *sparse_data_indice, *sparse_data_marker, *sparse_query_indice, *sparse_query_marker;
  float *sparse_query_val = new float[(size_t)(NUMQUERY) * DIMENSION];
	fvecs_yfcc_read_queries(QUERYFILE, DIMENSION, NUMQUERY, sparse_query_val);

  // LSH *hashFamily = new LSH(3, RANGE, MAXREPS, DIMENSION, sqrt(DIMENSION)); 
  LSH *hashFamily = new LSH(3, RANGE, MAXREPS, DIMENSION, DIMENSION); 
  unsigned int *all_hashes = new unsigned int [(size_t)NUMBASE * MAXREPS];
  unsigned int *indices_unused;

  size_t chunk_size = 1000000; // For now needs to be multiple of 1000000
  cout << NUMBASE << endl;
  for (size_t i = 0; i < (NUMBASE + chunk_size - 1) / chunk_size; i++) {
    size_t num_vectors = min(chunk_size, NUMBASE - i * chunk_size);
    cout << "Starting chunk " << i << ", contains " << num_vectors << " vectors." << endl;
    float *sparse_data_val_chunk = new float[(size_t)num_vectors * (size_t)DIMENSION];
    fvecs_yfcc_read_data(BASEFILE, i, num_vectors, sparse_data_val_chunk);
    hashFamily->getHash(all_hashes, indices_unused, 
                    sparse_data_indice, sparse_data_val_chunk, sparse_data_marker, 
                    num_vectors, 1, NUMBASE, i * chunk_size);
    delete[] sparse_data_val_chunk;
  }
#else
  int *sparse_data_indice;
  float *sparse_data_val;
  int *sparse_data_marker;
  int *sparse_query_indice;
  float *sparse_query_val;
  int *sparse_query_marker;
  readDataAndQueries(BASEFILE, NUMQUERY, NUMBASE, 
                     &sparse_data_indice, &sparse_data_val, &sparse_data_marker,
                     &sparse_query_indice, &sparse_query_val, &sparse_query_marker);
#endif

  // Read in ground truth
  unsigned int *gtruth_indice = new unsigned int[NUMQUERY * AVAILABLE_TOPK];
  float *gtruth_dist = new float[NUMQUERY * AVAILABLE_TOPK];
  readGroundTruthInt(GTRUTHINDICE, NUMQUERY, AVAILABLE_TOPK, gtruth_indice);
  readGroundTruthFloat(GTRUTHDIST, NUMQUERY, AVAILABLE_TOPK, gtruth_dist);


  end = Clock::now();
  etime_0 = (end - begin).count() / 1000000;

  // Generate hashes with maximum reps
  cout << "Starting total hash generation" << endl;

  cout << "Starting index building and query grid parameter test" << endl;
  unsigned int *queryOutputs = new unsigned int[NUMQUERY * TOPK]();
  if (!USE_FLINNG) {
    std::cout << "Using normal!" << std::endl;
    for (size_t REPS = 400; REPS <= MAXREPS; REPS *= 2) { 

      std::cout << "Initializing data hashes, array size " << REPS * NUMBASE << endl;
      // Initialize LSH hash.
      unsigned int *hashes;
      unsigned int *indices;
#ifdef YFCC
      hashes = all_hashes;
      hashFamily->set_reps(REPS);
#else
      LSH *hashFamily = new LSH(2, K, REPS, RANGE); 
      hashes = new unsigned int[REPS * NUMBASE];
      indices = new unsigned int[REPS * NUMBASE];
      hashFamily->getHash(hashes, indices, 
                          sparse_data_indice, sparse_data_val, sparse_data_marker, 
                          NUMBASE, 1, NUMBASE, 0);
#endif

      for (size_t RESERVOIR = 100; RESERVOIR <= 3200; RESERVOIR *= 2) {
        if (REPS * RESERVOIR < 128) {
          cout << "Skipping because too small\n";
          continue;
        }
        if ((REPS * RESERVOIR * (1 << RANGE)) > pow(10, 11)) {
          cout << "Skipping because too big\n";
          continue;
        }
        std::cout << "STATS_NORMAL: " << RESERVOIR << " " << RANGE << " "
                  << REPS << std::endl;
          do_normal(RESERVOIR, REPS, RANGE, hashes, indices, REPS, gtruth_indice,
                   gtruth_dist, sparse_query_indice, sparse_query_val, sparse_query_marker, hashFamily);
      }

#ifndef YFCC
      delete[] hashes;
      delete[] indices;
      delete hashFamily;
#endif
    }
  } else {
    std::cout << "Using groups!" << std::endl;
    for (size_t REPS = 200; REPS <= MAXREPS; REPS *= 2) { 

      std::cout << "Initializing data hashes, array size " << REPS * NUMBASE << endl;
      // Initialize LSH hash.
      unsigned int *hashes;
      unsigned int *indices;
#ifdef YFCC
      hashes = all_hashes;
      hashFamily->set_reps(REPS);
#else
      LSH *hashFamily = new LSH(2, K, REPS, RANGE); 
      hashes = new unsigned int[REPS * NUMBASE];
      indices = new unsigned int[REPS * NUMBASE];
      hashFamily->getHash(hashes, indices, 
                          sparse_data_indice, sparse_data_val, sparse_data_marker, 
                          NUMBASE, 1, NUMBASE, 0);
#endif

      for (size_t R = 2; R < 5; R++) {
        for (size_t B = 1 << 15; B < 1 << 25; B *= 2) {
          std::cout << "STATS_GROUPS: " << R << " " << B << " " << RANGE << " "
                    << REPS << std::endl;
          do_group(B, R, REPS, RANGE, hashes, REPS, gtruth_indice,
                   gtruth_dist, sparse_query_indice, sparse_query_val,
                   sparse_query_marker, hashFamily);
        }
      }
    
#ifndef YFCC
      delete[] hashes;
      delete[] indices;
      delete hashFamily;
#endif
    }
  }

  delete[] sparse_data_indice;
#ifndef YFCC
  delete[] sparse_data_val;
#endif
  delete[] sparse_data_marker;
  delete[] sparse_query_indice;
  delete[] sparse_query_val;
  delete[] sparse_query_marker;
  delete[] gtruth_indice;
  delete[] gtruth_dist;
  delete[] queryOutputs;
}
