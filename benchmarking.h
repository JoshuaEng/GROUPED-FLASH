
#pragma once

/* Select a dataset below by uncommenting it.
Then modify the file location and parameters below in the Parameters section. */

// Note that gtruthdist and gtruthindc require no new lines (afaik), so
// use a sed command to fix it, e.g.
// sed -i ':a;N;$!ba;s/\n/ /g' ../CalculateGroundTruth/proteomes_indices.txt 

// #define URL
// #define WEBSPAM_TRI
#define DNA_FULL_GENOME

#define USE_FLINNG false

/* Parameters. */
#if defined URL

#define SPARSEDATASET

#define NUMHASHBATCH				200

#define K					4
#define RANGE_POW				15
#define RANGE_ROW_U				15

#define RESERVOIR_SIZE				32

#define DIMENSION				120
#define FULL_DIMENSION				3231961
#define NUMBASE					2386130
#define MAX_RESERVOIR_RAND			2386130
#define NUMQUERY				10000
#define TOPK					128
#define AVAILABLE_TOPK				1024

#define NUMQUERY				10000
#define AVAILABLE_TOPK				1024
#define TOPK					128

#define BASEFILE		"../Data/Url/url_data"
#define GTRUTHINDICE	        "../Data/Url/url_gtruth_indices"
#define GTRUTHDIST		"../Data/Url/url_gtruth_distances"

#elif defined WEBSPAM_TRI

#define SPARSEDATASET

#define NUMHASHBATCH				50

#define K					4
#define RANGE_POW				18
#define RANGE_ROW_U				18

#define DIMENSION				4000
#define FULL_DIMENSION				16609143
#define NUMBASE					340000
#define MAX_RESERVOIR_RAND			35000
#define NUMQUERY				10000
#define TOPK					128
#define AVAILABLE_TOPK				1024

#define NUMQUERY				10000
#define AVAILABLE_TOPK				1024
#define TOPK					128

#define BASEFILE	        "../Data/Webspam/webspam_data"
#define GTRUTHINDICE	        "../Data/Webspam/webspam_gtruth_indices"
#define GTRUTHDIST		"../Data/Webspam/webspam_gtruth_distances"

#elif defined DNA_FULL_GENOME

#define SETDATASET

#define NUMHASHBATCH				50

#define K					1

#define NUMBASE					127219
#define NUMQUERY				10000
#define TOPK					128
#define AVAILABLE_TOPK				128

#define BASEFILE	        "../Data/Genomes/genomes_data"
#define GTRUTHINDICE	        "../Data/Genomes/genomes_gtruth_indices"
#define GTRUTHDIST		"../Data/Genomes/genomes_gtruth_distances"
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

