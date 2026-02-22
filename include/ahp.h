#include <matrix.h>
#include <math.h>
#include <numeric>

#define COMPUTE_GEOM_MEAN_WEIGHT_VECTOR

std::vector<double> getWeightVector(SquareMatrix mat);

double getConsistency(SquareMatrix mat, std::vector<double> weight);

double getConsistencyIndex(double consistency, size_t labels);