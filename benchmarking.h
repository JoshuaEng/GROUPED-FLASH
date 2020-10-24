
#pragma once

/* Select a dataset below by uncommenting it.
Then modify the file location and parameters below in the Parameters section. */

//#define SIFT1M
//#define URL
#define WEBSPAM_TRI
//#define KDD12

/* Parameters. */

#if defined SIFT1M

#define DENSE_DATASET

#define NUMHASHBATCH				100
#define BATCHPRINT					10

#define RANGE_POW					22
#define RANGE_ROW_U					18
#define SAMFACTOR					24
#define NUMTABLES					512
#define RESERVOIR_SIZE				32
#define OCCUPANCY					0.4

#define QUERYPROBES					1
#define HASHINGPROBES				1

#define DIMENSION					128
#define FULL_DIMENSION				128
#define NUMQUERY					10000
#define NUMBASE						1000000
#define MAX_RESERVOIR_RAND			100000

#define AVAILABLE_TOPK				1000
#define TOPK						128

#define BASEFILE		"../files/datasets/sift1m/sift_base.fvecs"
#define QUERYFILE		"../files/datasets/sift1m/sift_query.fvecs"
#define GTRUTHINDICE	"../files/datasets/sift1m/sift1m_gtruth_indices.txt"
#define GTRUTHDIST		"../files/datasets/sift1m/sift1m_gtruth_distances.txt"

#elif defined URL

#define SPARSE_DATASET

#define NUMHASHBATCH				200
#define BATCHPRINT					10

#define K							4
#define RANGE_POW					15
#define RANGE_ROW_U					15

#define NUMTABLES					128
#define RESERVOIR_SIZE				32
#define OCCUPANCY					1

#define QUERYPROBES					1
#define HASHINGPROBES				1

#define DIMENSION					120
#define FULL_DIMENSION				3231961
#define NUMBASE						2386130
#define MAX_RESERVOIR_RAND			2386130
#define NUMQUERY					10000
#define TOPK						128
#define AVAILABLE_TOPK				1024

#define NUMQUERY					10000
#define AVAILABLE_TOPK				1024
#define TOPK						128

#define BASEFILE		"../files/datasets/url/url_combined"
#define QUERYFILE		"../files/datasets/url/url_combined"
#define GTRUTHINDICE	"../files/datasets/url/url_gtruth_indices.txt"
#define GTRUTHDIST		"../files/datasets/url/url_gtruth_distances.txt"

#elif defined WEBSPAM_TRI

#define SPARSE_DATASET

#define NUMHASHBATCH				50
#define BATCHPRINT					5

#define K							4
#define RANGE_POW					15
#define RANGE_ROW_U					15

#define NUMTABLES					32
#define RESERVOIR_SIZE				64
#define OCCUPANCY					1

#define QUERYPROBES					1
#define HASHINGPROBES				1

#define DIMENSION					4000
#define FULL_DIMENSION				16609143
#define NUMBASE						340000
#define MAX_RESERVOIR_RAND			35000
#define NUMQUERY					10000
#define TOPK						128
#define AVAILABLE_TOPK				1024

#define NUMQUERY					10000
#define AVAILABLE_TOPK				1024
#define TOPK						128

#define BASEFILE		"../webspam_wc_normalized_trigram.svm"
#define QUERYFILE		"./webspam_wc_normalized_trigram.svm"
#define GTRUTHINDICE	"../webspam/webspam_tri_gtruth_indices.txt"
#define GTRUTHDIST		"../webspam/webspam_tri_gtruth_distances.txt"

#endif

void benchmark_kselect();
void benchmark_naiverp(int RANDPROJ_COMPRESS);
void benchmark_paragrid();
void benchmark_bruteforce();
void benchmark_ava();
void benchmark_friendster_quality();
void benchmark_sparse();
void benchmark_dense();
void benchmark_doph(int TEST_DOPH);
void benchmark_smartrp(int SMART_RP);

#if !defined (DENSE_DATASET)
#define SAMFACTOR 24 // DUMMY.
#endif

#if !defined (SPARSE_DATASET)
#define K 10 // DUMMY
#endif
