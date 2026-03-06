#include <matrix.h>

auto SquareMatrix::size() const -> size_t { return data_.size(); }

auto SquareMatrix::get(size_t r, size_t c) -> double &
{
    assert(r < size() && c < size() && "Invalid matrix access dimensions");

    return data_[r][c];
}
auto SquareMatrix::get(size_t r, size_t c) const -> double
{
    assert(r < size() && c < size() && "Invalid matrix access dimensions");

    return data_[r][c];
}
auto SquareMatrix::getRow(size_t r) const -> std::vector<double>
{
    assert(r < size() && "Row number too big!");

    return data_[r];
}
auto SquareMatrix::getColumn(size_t c) const -> std::vector<double>
{
    assert(c < size() && "Column number too big!");

    std::vector<double> ret{};
    ret.reserve(size());

    for (size_t i = 0; i < size(); ++i)
    {
        ret.push_back(data_.at(i).at(c));
    }

    return ret;
}

auto SquareMatrix::mul(const std::vector<double> &vec) const -> std::vector<double>
{
    assert(size() == vec.size() && "Incorrect dimensions");

    std::vector<double> ret(size(), 0);

    for (size_t row = 0; row < size(); ++row)
    {
        for (size_t column = 0; column < size(); ++column)
        {
            ret[row] += vec[row] * get(row, column);
        }
    }

    return ret;
}

auto SquareMatrix::OneMatrix(int size) -> SquareMatrix
{
    std::vector<std::vector<double>> ret(size);
    for (int idx = 0; idx < size; ++idx)
    {
        ret[idx] = std::vector<double>(size, 1);
    }
    return {ret};
}
