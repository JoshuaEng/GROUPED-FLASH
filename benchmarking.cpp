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

  uint start_offset = 0;
#ifdef QUERYFILE
  int *query_indice;
  float *query_val;
  int *query_marker;
  readGraphQueries(QUERYFILE, &query_indice, &query_val, &query_marker);
#else
  start_offset = NUMQUERY;
#endif

  auto begin = Clock::now();
  auto end = Clock::now();
  float etime_0;

  // Create index
  LSH *hashFamily = new LSH(2, K, REPS, RANGE); // Initialize LSH hash.
  FLING *fling = new FLING(R, B, hashFamily, RANGE, REPS, NUMBASE);
  int hash_chunk = NUMBASE / NUMHASHBATCH;

  // Populate index
  for (int b = 0; b < NUMHASHBATCH; b++) {
    cout << "Hash batch " << b << endl;
    fling->insert(hash_chunk, sparse_indice, sparse_val,
                  sparse_marker + b * hash_chunk + start_offset);
  }

  fling->finalize_construction();

  // Do queries
  std::cout << "Querying...\n";
  begin = Clock::now();
  for (int i = 0; i < NUMQUERY; i++) {
    uint32_t recall_buffer[TOPK];
#ifdef QUERYFILE
    fling->query(query_indice, query_val, query_marker + i, TOPK,
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

#ifdef QUERYFILE
  delete[] query_indice;
  delete[] query_marker;
  delete[] query_val;
#endif

  /* Quality evaluations. */
  const int nCnt = 10;
  int nList[nCnt] = {1, 10, 20, 30, 32, 40, 50, 64, 100, TOPK};
  const int gstdCnt = 8;
  float gstdVec[gstdCnt] = {0.95, 0.90, 0.85, 0.80, 0.75, 0.70, 0.65, 0.50};
  const int tstdCnt = 10;
  int tstdVec[tstdCnt] = {1, 10, 20, 30, 32, 40, 50, 64, 100, TOPK};
  if (!similarityMetric(sparse_indice, sparse_val, sparse_marker, sparse_indice,
                        sparse_val, sparse_marker + start_offset, queryOutputs,
                        gtruth_dist, NUMQUERY, TOPK, AVAILABLE_TOPK, nList,
                        nCnt, NUMBASE)) {
    delete hashFamily;
    delete fling;
    delete queryOutputs;
    return;
  }
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

  uint start_offset = 0;
#ifdef QUERYFILE
  int *query_indice;
  float *query_val;
  int *query_marker;
  readGraphQueries(QUERYFILE, &query_indice, &query_val, &query_marker);
#else
  start_offset = NUMQUERY;
#endif

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
                     sparse_marker + b * hash_chunk + start_offset);
  }

  std::cout << "Querying..." << endl;
  begin = Clock::now();
#ifdef QUERYFILE
  myReservoir->ann(NUMQUERY, query_indice, query_val, query_marker,
                   queryOutputs, TOPK);
#else
  myReservoir->ann(NUMQUERY, sparse_indice, sparse_val, sparse_marker,
                   queryOutputs, TOPK);
#endif
  end = Clock::now();
  etime_0 = (end - begin).count() / 1000000;
  std::cout << "Queried " << NUMQUERY << " datapoints, used " << etime_0
            << "ms." << endl;

#ifdef QUERYFILE
  delete[] query_indice;
  delete[] query_marker;
  delete[] query_val;
#endif

  /* Quality evaluations. */
  const int nCnt = 10;
  int nList[nCnt] = {1, 10, 20, 30, 32, 40, 50, 64, 100, TOPK};
  const int gstdCnt = 8;
  float gstdVec[gstdCnt] = {0.95, 0.90, 0.85, 0.80, 0.75, 0.70, 0.65, 0.50};
  const int tstdCnt = 10;
  int tstdVec[tstdCnt] = {1, 10, 20, 30, 32, 40, 50, 64, 100, TOPK};
  if (!similarityMetric(sparse_indice, sparse_val, sparse_marker, sparse_indice,
                        sparse_val, sparse_marker + start_offset, queryOutputs,
                        gtruth_dist, NUMQUERY, TOPK, AVAILABLE_TOPK, nList,
                        nCnt, NUMBASE)) {
    delete hashFamily;
    delete queryOutputs;
    delete myReservoir;
    return;
  }
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
#ifndef GRAPHDATASET
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
  int *sparse_indice;
  float *sparse_val;
  int *sparse_marker;
  readGraph(GTRUTH, &gtruth_indice, &gtruth_dist, BASEFILE, &sparse_indice,
            &sparse_val, &sparse_marker);
#endif
  end = Clock::now();
  etime_0 = (end - begin).count() / 1000000;

  unsigned int *queryOutputs = new unsigned int[NUMQUERY * TOPK]();
  if (!USE_GROUPS) {
    std::cout << "Using normal!" << std::endl;
    for (size_t REPS = 20; REPS <= 3050; REPS *= 1.5) {
      for (size_t RESERVOIR = 4; RESERVOIR <= 2000; RESERVOIR *= 1.5) {
        for (size_t RANGE = 17; RANGE <= 17; RANGE++) {
          // if (((1 << RANGE) * REPS * RESERVOIR) < (1 << 30)) {
            std::cout << "STATS_NORMAL: " << RESERVOIR << " " << RANGE << " "
                      << REPS << std::endl;
            do_normal(RESERVOIR, REPS, RANGE, sparse_indice, sparse_val,
                      sparse_marker, gtruth_indice, gtruth_dist);
          // }
        }
      }
    }
  } else {
    std::cout << "Using groups!" << std::endl;
    for (size_t REPS = 20; REPS <= 2560; REPS *= 2) {
      for (size_t R = 2; R < 6; R++) {
        for (size_t B = 2500; B <= 80000; B *= 2) {
          for (size_t RANGE = 17; RANGE <= 17; RANGE++) {
            std::cout << "STATS_GROUPS: " << R << " " << B << " " << RANGE
                      << " " << REPS << std::endl;
            do_group(B, R, REPS, RANGE, sparse_indice, sparse_val,
                      sparse_marker, gtruth_indice, gtruth_dist);
          }
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
