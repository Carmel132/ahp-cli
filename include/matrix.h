#include <vector>
#include <cassert>

class SquareMatrix {
    std::vector<std::vector<double>> data_;

public:
    SquareMatrix(std::vector<std::vector<double>> data) : data_{data} {};

    size_t size() const;

    double& get(size_t r, size_t c);
    double get(size_t r, size_t c) const; 

    std::vector<double> getRow(size_t r) const;
    std::vector<double> getColumn(size_t c) const;

    std::vector<double> mul(const std::vector<double>& vec) const;
};

