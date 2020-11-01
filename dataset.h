#pragma once
#include <iostream>

using namespace std;

void anshuReadSparse(string fileName, int *indices, int *markers, unsigned int n, unsigned int bufferlen);
void readSparse(string fileName, int offset, int n, int *indices, float *values, int *markers, unsigned int bufferlen);

void Read_MNIST(string fileName, int NumberOfImages, int DataOfAnImage, float *data);
void Read_MNISTLabel(string fileName, int numToRead, int *labels);

void fvecs_read(const std::string& file, int offset, int readsize, float *out);
void bvecs_read(const std::string& file, int offset, int readsize, float *out);

void readGroundTruthInt(const std::string& file, int numQueries, int availableTopK, unsigned int *out);
void readGroundTruthFloat(const std::string& file, int numQueries, int availableTopK, float *out);

void readGroundTruthGraph(const std::string& file, uint **gtruth_indice, float **gtruth_dist);
void readSparseGraph(const std::string& file, int **sparse_indice, float **sparse_dist, int **sparse_marker);