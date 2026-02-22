#include <ahp.h>

std::vector<double> getWeightVector(SquareMatrix mat)
{
    std::vector<double> ret(mat.size());
    

    #ifdef COMPUTE_GEOM_MEAN_WEIGHT_VECTOR 

    for (size_t row{}; row < mat.size(); ++row) {
        double prod = 1;

        for (size_t column{}; column < mat.size(); ++column) {
            prod *= mat.get(row, column);
        }
        ret[row] = std::pow(prod, 1./mat.size());
    }

    #else

    std::vector<double> columnSums(mat.size(), 0);
    for (size_t column{}; column < mat.size(); ++column) {
        double sum = 0;
        for (size_t row = 0; row < mat.size(); ++row) {
            ret[row] = 0;
            sum += mat.get(row, column);
        }
        for (size_t row = 0; row < mat.size(); ++row) {
            ret[row] += mat.get(row, column) / sum;
        }
        for (auto &v : ret) {
            v /= mat.size();
        }
    }
    #endif

    double total = std::accumulate(ret.begin(), ret.end(), 0.0);
    for (auto &v : ret) v /= total;

    return ret;
}

double getConsistency(SquareMatrix mat, std::vector<double> weight)
{
    assert(weight.size() == mat.size() && "Incorrect matrix dimensions");

    std::vector<double> prod = mat.mul(weight);
    double ret = 0.0;

    for (size_t i{}; i < mat.size(); ++i) {
        ret += prod.at(i) / weight.at(i);
    }
    ret /= mat.size();

    return ret;
}

double getConsistencyIndex(double consistency, size_t labels)
{
    return (consistency - labels)/(labels - 1);
}
