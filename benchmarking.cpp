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

void do_group(size_t B, size_t R, size_t REPS, size_t RANGE, int *sparse_indice,
              float *sparse_val, int *sparse_marker,
              unsigned int *gtruth_indice, float *gtruth_dist) {

  unsigned int *queryOutputs = new unsigned int[NUMQUERY * TOPK]();

  auto begin = Clock::now();
  auto end = Clock::now();
  float etime_0;

  // Create index
  LSH *hashFamily = new LSH(2, K, REPS, RANGE); // Initialize LSH hash.
  FLING *fling = new FLING(R, B, hashFamily, RANGE, REPS, NUMBASE);
  int hash_chunk = NUMBASE / NUMHASHBATCH;

  // Populate index
  for (int b = 0; b < NUMHASHBATCH; b++) {
#ifdef QUERYFILE
    fling->insert(hash_chunk, sparse_indice, sparse_val,
                  sparse_marker + b * hash_chunk);
#else
    fling->insert(hash_chunk, sparse_indice, sparse_val,
                  sparse_marker + b * hash_chunk + NUMQUERY);
#endif
  }

  fling->finalize_construction();

#ifdef QUERYFILE
	// TODO: Read in queries
#endif

  // Do queries
	std::cout << "Querying...\n";
  begin = Clock::now();
  for (int i = 0; i < NUMQUERY; i++) {
    uint32_t recall_buffer[TOPK];
#ifdef QUERYFILE
    // TODO: Change this to use read in queries
		fling->query(sparse_indice, sparse_val, sparse_marker + i, TOPK,
                 recall_buffer);
#else
    fling->query(sparse_indice, sparse_val, sparse_marker + i, TOPK,
                 recall_buffer);
#endif
    for (size_t j = 0; j < TOPK; j++) {
      queryOutputs[TOPK * i + j] = recall_buffer[j];
    }
  }
  end = Clock::now();
  etime_0 = (end - begin).count() / 1000000;
  std::cout << "Queried " << NUMQUERY << " datapoints, used " << etime_0
            << "ms. \n";

  /* Quality evaluations. */
  const int nCnt = 10;
  int nList[nCnt] = {1, 10, 20, 30, 32, 40, 50, 64, 100, TOPK};
  const int gstdCnt = 8;
  float gstdVec[gstdCnt] = {0.95, 0.90, 0.85, 0.80, 0.75, 0.70, 0.65, 0.50};
  const int tstdCnt = 10;
  int tstdVec[tstdCnt] = {1, 10, 20, 30, 32, 40, 50, 64, 100, TOPK};
  similarityMetric(sparse_indice, sparse_val, sparse_marker, sparse_indice,
                   sparse_val, sparse_marker + NUMQUERY, queryOutputs,
                   gtruth_dist, NUMQUERY, TOPK, AVAILABLE_TOPK, nList, nCnt);
  similarityOfData(gtruth_dist, NUMQUERY, TOPK, AVAILABLE_TOPK, nList, nCnt);
  evaluate(queryOutputs, NUMQUERY, TOPK, gtruth_indice, gtruth_dist,
           AVAILABLE_TOPK, gstdVec, gstdCnt, tstdVec, tstdCnt, nList,
           nCnt); // The number of n interested.

  delete hashFamily;
  delete fling;
  delete queryOutputs;
}

void do_normal(size_t RESERVOIR, size_t REPS, size_t RANGE, int *sparse_indice,
               float *sparse_val, int *sparse_marker,
               unsigned int *gtruth_indice, float *gtruth_dist) {

  unsigned int *queryOutputs = new unsigned int[NUMQUERY * TOPK]();

  auto begin = Clock::now();
  auto end = Clock::now();
  float etime_0;

  LSH *hashFamily = new LSH(2, K, REPS, RANGE); // Initialize LSH hash.
  // Dimension not used so just pass in -1
  LSHReservoirSampler *myReservoir = new LSHReservoirSampler(
      hashFamily, RANGE, REPS, RESERVOIR, -1, RANGE, NUMBASE, QUERYPROBES,
      HASHINGPROBES,
      OCCUPANCY); // Initialize hashtables and other datastructures.

  int hash_chunk = NUMBASE / NUMHASHBATCH;
  for (int b = 0; b < NUMHASHBATCH; b++) {
    myReservoir->add(hash_chunk, sparse_indice, sparse_val,
                     sparse_marker + b * hash_chunk + NUMQUERY);
  }

  std::cout << "Querying...\n";
  begin = Clock::now();
  myReservoir->ann(NUMQUERY, sparse_indice, sparse_val, sparse_marker,
                   queryOutputs, TOPK);
  end = Clock::now();
  etime_0 = (end - begin).count() / 1000000;
  std::cout << "Queried " << NUMQUERY << " datapoints, used " << etime_0
            << "ms. \n";

  /* Quality evaluations. */
  const int nCnt = 10;
  int nList[nCnt] = {1, 10, 20, 30, 32, 40, 50, 64, 100, TOPK};
  const int gstdCnt = 8;
  float gstdVec[gstdCnt] = {0.95, 0.90, 0.85, 0.80, 0.75, 0.70, 0.65, 0.50};
  const int tstdCnt = 10;
  int tstdVec[tstdCnt] = {1, 10, 20, 30, 32, 40, 50, 64, 100, TOPK};
  similarityMetric(sparse_indice, sparse_val, sparse_marker, sparse_indice,
                   sparse_val, sparse_marker + NUMQUERY, queryOutputs,
                   gtruth_dist, NUMQUERY, TOPK, AVAILABLE_TOPK, nList, nCnt);
  similarityOfData(gtruth_dist, NUMQUERY, TOPK, AVAILABLE_TOPK, nList, nCnt);
  evaluate(queryOutputs, NUMQUERY, TOPK, gtruth_indice, gtruth_dist,
           AVAILABLE_TOPK, gstdVec, gstdCnt, tstdVec, tstdCnt, nList,
           nCnt); // The number of n interested.

  delete hashFamily;
  delete queryOutputs;
  delete myReservoir;
}

void benchmark_sparse() {
  float etime_0, etime_1, etime_2;
  auto begin = Clock::now();
  auto end = Clock::now();

  std::cout << "Reading groundtruth and data ... " << std::endl;
  begin = Clock::now();
#ifndef GPLUS
  unsigned int *gtruth_indice = new unsigned int[NUMQUERY * AVAILABLE_TOPK];
  float *gtruth_dist = new float[NUMQUERY * AVAILABLE_TOPK];
  readGroundTruthInt(GTRUTHINDICE, NUMQUERY, AVAILABLE_TOPK, gtruth_indice);
  readGroundTruthFloat(GTRUTHDIST, NUMQUERY, AVAILABLE_TOPK, gtruth_dist);

  int *sparse_indice = new int[(unsigned)((NUMBASE + NUMQUERY) * DIMENSION)];
  float *sparse_val = new float[(unsigned)((NUMBASE + NUMQUERY) * DIMENSION)];
  int *sparse_marker = new int[(NUMBASE + NUMQUERY) + 1];
  readSparse(BASEFILE, 0, (unsigned)(NUMBASE + NUMQUERY), sparse_indice,
             sparse_val, sparse_marker,
             (unsigned)((NUMBASE + NUMQUERY) * DIMENSION));
#else
  unsigned int *gtruth_indice;
  float *gtruth_dist;
  readGroundTruthGraph(GTRUTH, &gtruth_indice, &gtruth_dist);

  int *sparse_indice;
  float *sparse_val;
  int *sparse_marker;
  readSparseGraph(BASEFILE, &sparse_indice, &sparse_val, &sparse_marker);
#endif
  end = Clock::now();
  etime_0 = (end - begin).count() / 1000000;

  unsigned int *queryOutputs = new unsigned int[NUMQUERY * TOPK]();
  if (!USE_GROUPS) {
    std::cout << "Using normal!" << std::endl;
    for (size_t REPS = 20; REPS <= 640; REPS *= 2) {
      for (size_t RESERVOIR = 64; RESERVOIR <= 64 * 4; RESERVOIR *= 2) {
        std::cout << "STATS_NORMAL: " << RESERVOIR << " " << 15 << " " << REPS
                  << std::endl;
        do_normal(RESERVOIR, REPS, 15, sparse_indice, sparse_val, sparse_marker,
                  gtruth_indice, gtruth_dist);
      }
    }
  } else {
    std::cout << "Using groups!" << std::endl;
    for (size_t R = 3; R < 4; R++) {
      for (size_t B = 10000; B <= 40000; B *= 2) {
        for (size_t REPS = 20; REPS <= 640; REPS *= 2) {
          std::cout << "STATS_GROUPS: " << R << " " << B << " " << 15 << " "
                    << REPS << std::endl;
          do_group(B, R, REPS, 17, sparse_indice, sparse_val, sparse_marker,
                   gtruth_indice, gtruth_dist);
        }
      }
    }
  }

  delete[] sparse_indice;
  delete[] sparse_val;
  delete[] sparse_marker;
  delete[] gtruth_indice;
  delete[] gtruth_dist;
  delete[] queryOutputs;
}
