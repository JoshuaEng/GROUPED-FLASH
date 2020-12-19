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

void do_group(size_t B, size_t R, size_t REPS, size_t RANGE, uint *hashes,
              uint max_reps, unsigned int *gtruth_indice, float *gtruth_dist,
              int *query_sparse_indice, float *query_sparse_val, int *query_sparse_marker, 
              LSH *hash_family) {

  uint offset = 0;
#ifndef QUERYFILE
  offset = NUMQUERY;
#endif

  unsigned int *queryOutputs = new unsigned int[NUMQUERY * TOPK]();

  auto begin = Clock::now();
  auto end = Clock::now();
  float etime_0;

  // Create index
  FLING *fling =
      new FLING(R, B, hashes, max_reps, hash_family, RANGE, REPS, NUMBASE - offset);
  fling->finalize_construction();

  // Do queries
  std::cout << "Querying...\n";
  begin = Clock::now();
  for (uint i = 0; i < NUMQUERY; i++) {
    uint32_t recall_buffer[TOPK];
    fling->query(query_sparse_indice, query_sparse_val, query_sparse_marker + i, TOPK, recall_buffer);
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

  // TODO: Fix this for graphs
#ifndef QUERYFILE
  if (!similarityMetric(query_sparse_indice, query_sparse_val,
                        query_sparse_marker, query_sparse_indice,
                        query_sparse_val, query_sparse_marker + NUMQUERY,
                        queryOutputs, gtruth_dist, NUMQUERY, TOPK,
                        AVAILABLE_TOPK, nList, nCnt, NUMBASE)) {
    delete fling;
    delete queryOutputs;
    return;
  }
#endif

  similarityOfData(gtruth_dist, NUMQUERY, TOPK, AVAILABLE_TOPK, nList, nCnt);
  evaluate(queryOutputs, NUMQUERY, TOPK, gtruth_indice, gtruth_dist,
           AVAILABLE_TOPK, gstdVec, gstdCnt, tstdVec, tstdCnt, nList,
           nCnt); // The number of n interested.

  delete fling;
  delete queryOutputs;
}


void do_normal(size_t RESERVOIR, size_t REPS, size_t RANGE, uint *hashes, uint *indices, 
              uint max_reps, unsigned int *gtruth_indice, float *gtruth_dist,
              int *query_sparse_indice, float *query_sparse_val, int *query_sparse_marker, 
              LSH *hashFamily) {

  unsigned int *queryOutputs = new unsigned int[NUMQUERY * TOPK]();

  uint start_offset = 0;
#ifndef QUERYFILE
  start_offset = NUMQUERY;
#endif

  auto begin = Clock::now();
  auto end = Clock::now();
  float etime_0;

  // Dimension not used so just pass in -1
  LSHReservoirSampler *myReservoir = new LSHReservoirSampler(
      hashFamily, RANGE, REPS, RESERVOIR, -1, RANGE, NUMBASE, QUERYPROBES,
      HASHINGPROBES,
      OCCUPANCY); // Initialize hashtables and other datastructures.



  for (size_t start = 0; start < NUMBASE - start_offset;) {
    size_t end = min(start + NUMBASE / NUMHASHBATCH, (size_t)NUMBASE - start_offset);
    auto indexFunc = [start_offset, start](size_t table, size_t probe) { 
      return (table * (size_t) (NUMBASE - start_offset) + start + probe); 
    };
    myReservoir->add(end - start, hashes, indices, indexFunc);
    start = end;
  }

  std::cout << "Querying..." << endl;
  begin = Clock::now();
  myReservoir->ann(NUMQUERY, query_sparse_indice, query_sparse_val, query_sparse_marker,
                   queryOutputs, TOPK);
  end = Clock::now();
  etime_0 = (end - begin).count() / 1000000;
  std::cout << "Queried " << NUMQUERY << " datapoints, used " << etime_0
            << "ms." << endl;
 
  /* Quality evaluations. */
  const int nCnt = 10;
  int nList[nCnt] = {1, 10, 20, 30, 32, 40, 50, 64, 100, TOPK};
  const int gstdCnt = 8;
  float gstdVec[gstdCnt] = {0.95, 0.90, 0.85, 0.80, 0.75, 0.70, 0.65, 0.50};
  const int tstdCnt = 10;
  int tstdVec[tstdCnt] = {1, 10, 20, 30, 32, 40, 50, 64, 100, TOPK};

#ifndef QUERYFILE
  // TODO: Fix this for graphs
  if (!similarityMetric(query_sparse_indice, query_sparse_val,
                        query_sparse_marker, query_sparse_indice,
                        query_sparse_val, query_sparse_marker + NUMQUERY,
                        queryOutputs, gtruth_dist, NUMQUERY, TOPK,
                        AVAILABLE_TOPK, nList, nCnt, NUMBASE)) {
    delete myReservoir;
    delete queryOutputs;
    return;
  }
#endif

  similarityOfData(gtruth_dist, NUMQUERY, TOPK, AVAILABLE_TOPK, nList, nCnt);
  evaluate(queryOutputs, NUMQUERY, TOPK, gtruth_indice, gtruth_dist,
           AVAILABLE_TOPK, gstdVec, gstdCnt, tstdVec, tstdCnt, nList,
           nCnt); // The number of n interested.

  delete myReservoir;
  delete queryOutputs;
}

void benchmark_sparse() {

  omp_set_num_threads(20);

  float etime_0, etime_1, etime_2;
  auto begin = Clock::now();
  auto end = Clock::now();

  std::cout << "Reading groundtruth and data ... " << std::endl;
  begin = Clock::now();

// Read in data
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
            
  uint start_offset = NUMQUERY;

#else
  unsigned int *gtruth_indice;
  float *gtruth_dist;
  int *sparse_indice;
  float *sparse_val;
  int *sparse_marker;
  readGraph(GTRUTH, &gtruth_indice, &gtruth_dist, BASEFILE, &sparse_indice,
            &sparse_val, &sparse_marker);

  int *query_sparse_indice;
  float *query_val;
  int *query_marker;
  readGraphQueries(QUERYFILE, &query_sparse_indice, &query_val, &query_marker);
  
  uint start_offset = 0;

#endif

  end = Clock::now();
  etime_0 = (end - begin).count() / 1000000;

  // Generate hashes with maximum reps
  cout << "Starting total hash generation" << endl;
  auto RANGE = 17;

  cout << "Starting index building and query grid parameter test" << endl;
  unsigned int *queryOutputs = new unsigned int[NUMQUERY * TOPK]();
  if (!USE_GROUPS) {
    std::cout << "Using normal!" << std::endl;
    for (size_t REPS = 6; REPS <= 3050; REPS *= 1.5) {

      std::cout << "Initializing data hashes, array size " << REPS * (NUMBASE - start_offset) << endl;
      LSH *hashFamily = new LSH(2, K, REPS, RANGE); // Initialize LSH hash.
      unsigned int *hashes =
          new unsigned int[REPS * (NUMBASE - start_offset)];
      unsigned int *indices =
          new unsigned int[REPS * (NUMBASE - start_offset)];
      hashFamily->getHash(hashes, indices, sparse_indice, sparse_val,
                          sparse_marker + start_offset, NUMBASE - start_offset,
                          1);

      for (size_t RESERVOIR = 6; RESERVOIR <= 2000; RESERVOIR *= 1.5) {
      // for (size_t RESERVOIR = 6; RESERVOIR <= 2000; RESERVOIR *= 1.5) {
        // if (((1 << RANGE) * REPS * RESERVOIR) < (1 << 30)) {
        if (REPS * RESERVOIR > 1000000) {
          continue;
        }
        std::cout << "STATS_NORMAL: " << RESERVOIR << " " << RANGE << " "
                  << REPS << std::endl;
#ifdef QUERYFILE
          do_normal(RESERVOIR, REPS, RANGE, hashes, indices, REPS, gtruth_indice,
                   gtruth_dist, query_sparse_indice, query_val, 
                   query_marker, hashFamily);
#else
          do_normal(RESERVOIR, REPS, RANGE, hashes, indices, REPS, gtruth_indice,
                   gtruth_dist, sparse_indice, sparse_val,
                   sparse_marker, hashFamily);
#endif
        // }
      }

      delete[] hashes;
      delete[] indices;
      delete hashFamily;
    }
  } else {
    std::cout << "Using groups!" << std::endl;
    for (size_t REPS = 20; REPS <= 2560; REPS *= 2) {

      std::cout << "Initializing data hashes, array size " << REPS * (NUMBASE - start_offset) << endl;
      LSH *hashFamily = new LSH(2, K, REPS, RANGE); // Initialize LSH hash.
      unsigned int *hashes =
          new unsigned int[REPS * (NUMBASE - start_offset)];
      unsigned int *indices =
          new unsigned int[REPS * (NUMBASE - start_offset)];
      hashFamily->getHash(hashes, indices, sparse_indice, sparse_val,
                          sparse_marker + start_offset, NUMBASE - start_offset,
                          1);

      std::cout << "Initializing query hashes, array size " << REPS * NUMQUERY << endl;

      for (size_t R = 2; R < 6; R++) {
        for (size_t B = 2000; B * R <= 1 << 16; B *= 2) {
          std::cout << "STATS_GROUPS: " << R << " " << B << " " << RANGE << " "
                    << REPS << std::endl;
#ifdef QUERYFILE
          do_group(B, R, REPS, RANGE, hashes, REPS, gtruth_indice,
                   gtruth_dist,  query_sparse_indice, query_val, 
                   query_marker, hashFamily);
#else
          do_group(B, R, REPS, RANGE, hashes, REPS, gtruth_indice,
                   gtruth_dist, sparse_indice, sparse_val,
                   sparse_marker, hashFamily);
#endif
        }
      }
    
    delete[] hashes;
    delete[] indices;
    delete hashFamily;
    }
  }

  delete[] sparse_indice;
  delete[] sparse_val;
  delete[] sparse_marker;
  delete[] gtruth_indice;
  delete[] gtruth_dist;
  delete[] queryOutputs;
}
