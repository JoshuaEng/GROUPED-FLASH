
#pragma once

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

/* Select a dataset below by uncommenting it.
Then modify the file location and parameters below in the Parameters section. */

// Note that gtruthdist and gtruthindc require there to be no new lines (afaik), so
// replace new lines with spaces in those files

// #define URL
// #define WEBSPAM_TRI
// #define DNA_FULL_GENOME
// #define PROMETHION_SHORT
#define YFCC
// #define DNA_FULL_PROTEOME

#define USE_FLINNG true

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

#define BASEFILE		"../Data/Url/data"
#define GTRUTHINDICE	        "../Data/Url/indices"
#define GTRUTHDIST		"../Data/Url/distances"

#elif defined WEBSPAM_TRI

#define SPARSEDATASET

#define NUMHASHBATCH				50

#define K					4
#define RANGE_POW				18
#define RANGE_ROW_U				18

#define DIMENSION				4000
#define FULL_DIMENSION				16609143
#define NUMBASE					340000
#define NUMQUERY				10000
#define TOPK					128
#define AVAILABLE_TOPK				1024

#define NUMQUERY				10000
#define AVAILABLE_TOPK				1024
#define TOPK					128

#define BASEFILE	        "../Data/Webspam/data"
#define GTRUTHINDICE	        "../Data/Webspam/indices"
#define GTRUTHDIST		"../Data/Webspam/distances"

#elif defined DNA_FULL_GENOME

#define SETDATASET

#define NUMHASHBATCH				50

#define K					1

#define NUMBASE					117219
#define NUMQUERY				10000
#define TOPK					128
#define AVAILABLE_TOPK				128

#define BASEFILE	        "../Data/Genomes/data"
#define GTRUTHINDICE	        "../Data/Genomes/indices"
#define GTRUTHDIST		"../Data/Genomes/distances"

#elif defined DNA_FULL_PROTEOME

#define SETDATASET

#define NUMHASHBATCH				50

#define K					1

#define NUMBASE					116373
#define NUMQUERY				10000
#define TOPK					128
#define AVAILABLE_TOPK				128

#define BASEFILE	        "../Data/Proteomes/data"
#define GTRUTHINDICE	        "../Data/Proteomes/indices"
#define GTRUTHDIST		"../Data/Proteomes/distances"


#elif defined PROMETHION_SHORT

#define SETDATASET

#define NUMHASHBATCH				200

#define K					1

#define NUMBASE					3696341
#define NUMQUERY				10000
#define TOPK					128
#define AVAILABLE_TOPK				128

#define BASEFILE	        "../Data/Promethion/data"
#define GTRUTHINDICE	        "../Data/Promethion/indices"
#define GTRUTHDIST		"../Data/Promethion/distances"

#elif defined YFCC 

#define DENSEDATASET

// #define NUMBASE                                 96970001
#define NUMBASE                                 8000000
#define NUMQUERY				10000
#define TOPK					128
#define AVAILABLE_TOPK				100
#define NUMHASHBATCH				200
#define MAXREPS                                 1600

#define DIMENSION				4096
#define RANGE   				12

// #define BASEFILE	        "/home/jae4/Data/Yffc/YFCC100M_hybridCNN_gmean_fc6_"
// #define GTRUTHINDICE	        "/home/jae4/Data/Yffc/yfcc100m_nq10000_k100_indices_" STR(NUMBASE) ".txt"
// #define GTRUTHDIST		"/home/jae4/Data/Yffc/yfcc100m_nq10000_k100_distances.txt"
// #define QUERYFILE	        "/home/jae4/Data/Yffc/yfcc100m_nq10000_queries.txt"
#define BASEFILE                "/scratch0/jae4/Intel/YFCC100M_hybridCNN_gmean_fc6_"
#define GTRUTHINDICE            "/scratch0/jae4/Intel/yfcc100m_nq10000_k100_indices.txt"
#define GTRUTHDIST              "/scratch0/jae4/Intel/yfcc100m_nq10000_k100_distances.txt"
#define QUERYFILE               "/scratch0/jae4/Intel/yfcc100m_nq10000_queries.txt"


#endif

void benchmark_sparse();


