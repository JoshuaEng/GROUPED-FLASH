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


  myReservoir->add(NUMBASE - start_offset, hashes, indices);

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
  float etime_0, etime_1, etime_2;
  auto begin = Clock::now();
  auto end = Clock::now();

  std::cout << "Reading groundtruth and data ... " << std::endl;

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

  auto sizes = new uint[NUMBASE];
  for (uint i = 0; i < NUMBASE; i++) {
    sizes[i] = sparse_marker[i + 1] - sparse_marker[i];
  }

  cout << "Starting query" << endl;
  begin = Clock::now();
  for (uint i = 0; i < NUMQUERY; i++) {
    uint start = query_marker[i];
    uint end = query_marker[i + 1];
    uint size = end - start;
    vector<uint> all_points(0);
    for (uint j = start; j < end; j++) {
      uint graph_start = sparse_marker[query_sparse_indice[j]];
      uint graph_end = sparse_marker[query_sparse_indice[j] + 1];
      for (uint k = graph_start; k < graph_end; k++) {
        all_points.push_back(sparse_indice[k]);
      }
    }
    sort(all_points.begin(), all_points.end());

    vector<pair<float, uint>> pairs(0);
    uint current = all_points[0];
    uint current_count = 0;
    for (uint observation : all_points) {
      if (observation == current) {
        current_count++;
      }
      else {
        pairs.push_back(pair<float, uint>(current_count / (float)(size + sizes[current] - current_count), current));
        current_count = 1;
        current = observation;
      }
    }
    sort(pairs.begin(), pairs.end());

    // Comment this out if you want to test speed as the printing takes a bit
    // cout << i << ": ";
    // for (int j = pairs.size() - 1; j >= 0; j--) {
    //   cout << pairs[j].second << " ";
    // }
    // cout << "\n";
    // cout << i << ": ";
    // for (int j = pairs.size() - 1; j >= 0; j--) {
    //   cout << pairs[j].first << " ";
    // }
    // cout << "\n";
  }

  end = Clock::now();
  cout <<  "Took: " << (end - begin).count() / 1000000 << endl;
}
