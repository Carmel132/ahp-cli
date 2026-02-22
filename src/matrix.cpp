#include <matrix.h>

size_t SquareMatrix::size() const {return data_.size();}

double& SquareMatrix::get(size_t r, size_t c) {
    assert(r < size() && c < size() && "Invalid matrix access dimensions");

    return data_[r][c];
}
double SquareMatrix::get(size_t r, size_t c) const {
    assert(r < size() && c < size() && "Invalid matrix access dimensions");

    return data_[r][c];
}
std::vector<double> SquareMatrix::getRow(size_t r) const {
    assert(r < size() && "Row number too big!");
    
    return data_[r];
}
std::vector<double> SquareMatrix::getColumn(size_t c) const {
    assert(c < size() && "Column number too big!");

    std::vector<double> ret{};
    ret.reserve(size());

    for (size_t i = 0; i < size(); ++i) {
        ret.push_back(data_.at(i).at(c));
    }

    return ret;
}

std::vector<double> SquareMatrix::mul(const std::vector<double> &vec) const
{
    assert(size() == vec.size() && "Incorrect dimensions");

    std::vector<double> ret(size(), 0);

    for (size_t row = 0; row < size(); ++row) {
        for (size_t column = 0; column < size(); ++column) {
            ret[row] += vec[row] * get(row, column);
        }
    }

    return ret;
}
