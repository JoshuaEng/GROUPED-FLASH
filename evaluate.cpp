#include "evaluate.h"
#include "misc.h"

#include <iostream>

using namespace std;

/*
* Function:  evaluate
* --------------------
* Evaluate the results of a dataset using various metrics, prints the result
*
*  returns: nothing
*/
void evaluate (
	unsigned int *queryOutputs,		// The output indices of queries.
	int numQueries,			// The number of query entries, should be the same for outputs and groundtruths.
	int topk,				// The topk per query contained in the queryOutputs.
	unsigned int *groundTruthIdx,	// The groundtruth indice vector.
	float *groundTruthDist,	// The groundtruth distance vector.
	int availableTopk		// Available topk information in the groundtruth.
	) {				// The number of n(s) interested.

	r1Metric(queryOutputs, numQueries, topk, groundTruthIdx, availableTopk);
	r10Metric(queryOutputs, numQueries, topk, groundTruthIdx, availableTopk);
	
	// gMetric(queryOutputs, numQueries, topk, groundTruthIdx, groundTruthDist, availableTopk, gstdVec, gstdCnt, nList, nCnt);
}

// Accuracy measure R@k: fraction of query where the nearest neighbor is in the top k result.
void r1Metric(unsigned int *queryOutputs, int numQueries, int topk,
	unsigned int *groundTruthIdx, int availableTopk) {

	printf("\nR1@k: Average fraction of query where the nearest neighbor is in the k first results. \n");


	int *good_counts = new int[topk]();

	unsigned int top_nn;

	for (int i = 0; i < numQueries; i++) {

		top_nn = groundTruthIdx[i * availableTopk];

		for (int j = 0; j < topk; j++) { // Look for top-1 in top-k.

			if (top_nn == queryOutputs[i * topk + j]) {  // When top-1 is found.
				for (int goodN = j; goodN < topk; goodN++) {	 // For each standard.
					good_counts[goodN]++; // Count this query.
				}
				break; // Force goto next query to ensure testing integrity. .
			}

		}
	}

	for (int myN = 0; myN < topk; myN++) {
		printf("R%d@%d = %1.3f \n", 1, myN + 1, (float)good_counts[myN] / numQueries);
	}
	delete[] good_counts;
}

void tMetric(unsigned int *queryOutputs, int numQueries, int topk,
	unsigned int *groundTruthIdx, float *groundTruthDist, int availableTopk, int *tstdVec, const int tstdCnt) {

	printf("\nT@k Average fraction of top k nearest neighbors returned in k first results. \n");

	float *sumOfFraction = new float[tstdCnt]();

	for (int g = 0; g < tstdCnt; g++) { // For each test.

		for (int i = 0; i < numQueries; i++) {

			unordered_set<unsigned int> topTGtruths(groundTruthIdx + i * availableTopk, groundTruthIdx + i * availableTopk + tstdVec[g]);
			unordered_set<unsigned int> topTOutputs(queryOutputs + i * topk, queryOutputs + i * topk + tstdVec[g]);

			float tmp = 0;
			for (const auto& elem : topTGtruths) {
				if (topTOutputs.find(elem) != topTOutputs.end()) { // If elem is found in the intersection.
					tmp++;
				}
			}
			sumOfFraction[g] += tmp / (float)tstdVec[g];
		}
	}

	for (int g = 0; g < tstdCnt; g++) {
		printf("T@%d = %1.3f\n", tstdVec[g],
			(float)sumOfFraction[g] / (float)numQueries);
	}
	for (int g = 0; g < tstdCnt; g++) printf("%d ", tstdVec[g]);
	printf("\n");
	for (int g = 0; g < tstdCnt; g++) printf("%1.3f ", (float)sumOfFraction[g] / (float)numQueries);
	printf("\n"); printf("\n");

	delete[] sumOfFraction;

}

void r10Metric(unsigned int *queryOutputs, int numQueries, int topk,
	unsigned int *groundTruthIdx, int availableTopk) {

	printf("\nR10@k Average fraction of top 10 nearest neighbors returned in k first results. \n");

	int *good_counts = new int[topk]();

	for (int i = 0; i < numQueries; i++) {
		unordered_set<unsigned int> topGtruths(groundTruthIdx + i * availableTopk, groundTruthIdx + i * availableTopk + 10);
		for (int denominator = 10; denominator <= topk; denominator++) {
			unordered_set<unsigned int> topOutputs(queryOutputs + i * topk, queryOutputs + i * topk + denominator);
			for (const auto& elem : topGtruths) {
				if (topOutputs.find(elem) != topOutputs.end()) { // If elem is found in the intersection.
					good_counts[denominator - 10]++;
				}
			}
		}
	}

	for (int denominator = 10; denominator <= topk; denominator++) {
		printf("R%d@%d = %1.3f \n", 10, denominator, (float)good_counts[denominator - 10] / numQueries / 10);
	}
	printf("\n"); printf("\n");

	delete[] good_counts;

}

