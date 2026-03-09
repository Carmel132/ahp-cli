#pragma once

#include <cassert>
#include <utility>
#include <vector>

class SquareMatrix
{
    std::vector<std::vector<double>> data_;

public:
    SquareMatrix(std::vector<std::vector<double>> data) : data_{std::move(std::move(data))} {};

    SquareMatrix() : data_{} {};

    [[nodiscard]] auto size() const -> size_t;

    auto get(size_t row, size_t column) -> double &;
    [[nodiscard]] auto get(size_t row, size_t column) const -> double;

    [[nodiscard]] auto getRow(size_t row) const -> std::vector<double>;
    [[nodiscard]] auto getColumn(size_t column) const -> std::vector<double>;

    [[nodiscard]] auto mul(const std::vector<double> &vec) const -> std::vector<double>;

    std::vector<std::vector<double>> getUnderlying() const;

    static auto OneMatrix(int size) -> SquareMatrix;
};
