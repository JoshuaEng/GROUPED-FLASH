#include <cmath>

#include "LSHReservoirSampler.h"
#include "dataset.h"
#include "misc.h"
#include "evaluate.h"
#include "indexing.h"
#include "omp.h"
#include "MatMul.h"
#include "benchmarking.h"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include "FrequentItems.h"
#include "dataset.h"
#include "benchmarking.h"
#include "HybridCNNReader.h"

#include <iostream>
using namespace std;

float get_mag(float *start) {
	float mag = 0;
	for (size_t d = 0; d < DIMENSION; d++) {
		float val = start[d]; 
		mag += val * val;
	}
	return sqrt(mag);
}

float get_distance(float *vec1, float *vec2) {
	float mag = 0;
	for (size_t d = 0; d < DIMENSION; d++) {
		float val = vec1[d] - vec2[d]; 
		mag += val * val;
	}
	return sqrt(mag);
}

void compute_groundtruth_yfcc() {

	omp_set_num_threads(40);

  cout << "Computing ground truth for " << NUMBASE << " first points." << endl;

	size_t topk = 128;

	vector<priority_queue<pair<float, size_t>>> queues(0);
	for (size_t q = 0; q < NUMQUERY; q++) {
		queues.push_back(priority_queue<pair<float, size_t>>());
	}

  float *queries = new float[(size_t)(NUMQUERY) * DIMENSION];
  fvecs_yfcc_read_queries(QUERYFILE, DIMENSION, NUMQUERY, queries);
  
  // float queries_mag[NUMQUERY];
  // for (size_t q = 0; q < NUMQUERY; q++) {
	//   queries_mag[q] = get_mag(queries + q * DIMENSION);
  // }
  
  int batch = 1000;
  float* fvs = new float[DIMENSION * batch];
  long* ids  = new long[batch];        // not used
	size_t print_freq = 10;
	size_t so_far = 0;

  BinaryReader reader(BASEFILE);
  size_t features_read = 0;
  double totals[DIMENSION + 1] = {};
	auto begin = Clock::now();
  while (features_read < NUMBASE) {
		if (so_far % print_freq == 0) {
			auto etime_0 = (Clock::now() - begin).count() / 1000000;
			cout << (float)features_read * 100 / NUMBASE << "% done, used " << etime_0 / float(1000) << " seconds." << endl;
		}
		so_far++;
		size_t read = reader.read(batch, fvs, DIMENSION*batch, ids, batch);
		float data_mag[read];

// #pragma omp parallel for
// 		for (size_t point = 0; point < read; point++) {
// 			data_mag[point] = get_mag(fvs + point * DIMENSION);
// 		}

#pragma omp parallel for
		for (size_t q = 0; q < NUMQUERY; q++) {
			for (size_t point = 0; point < read; point++) {
				float distance = get_distance(fvs + point * DIMENSION, queries + q * DIMENSION);
				if (queues[q].size() < topk) {
					queues[q].push(make_pair(distance, point));
				} else {
					if (queues[q].top().first > distance) {
						queues[q].pop();
						queues[q].push(make_pair(distance, point + features_read));
					}
				}
			}
		}	
		features_read += read;
  }

	cout << endl;
	for (size_t q = 0; q < NUMQUERY; q++) {
		vector<float> dist_buffer(0);
		vector<size_t> index_buffer(0);
		while (queues[q].size() > 0) {
			dist_buffer.push_back(queues[q].top().first);
			index_buffer.push_back(queues[q].top().second);
			queues[q].pop();
		}
		for (int i = dist_buffer.size() - 1; i >= 0; i--) {
			cout << dist_buffer[i] << " ";
		}
		cout << endl;
		for (int i = index_buffer.size() - 1; i >= 0; i--) {
			cout << index_buffer[i] << " ";
		}
		cout << endl;
	}

}


int main() {
	benchmark_sparse();
	// compute_groundtruth_yfcc();
}
