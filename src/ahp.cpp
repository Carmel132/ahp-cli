#include <ahp.h>

auto getWeightVector(const SquareMatrix &mat) -> std::vector<double>
{
    std::vector<double> ret(mat.size());

#ifdef COMPUTE_GEOM_MEAN_WEIGHT_VECTOR

    for (size_t row{}; row < mat.size(); ++row)
    {
        double prod = 1;

        for (size_t column{}; column < mat.size(); ++column)
        {
            prod *= mat.get(row, column);
        }
        ret[row] = std::pow(prod, 1. / static_cast<double>(mat.size()));
    }

#else

    std::vector<double> columnSums(mat.size(), 0);
    for (size_t column{}; column < mat.size(); ++column)
    {
        double sum = 0;
        for (size_t row = 0; row < mat.size(); ++row)
        {
            ret[row] = 0;
            sum += mat.get(row, column);
        }
        for (size_t row = 0; row < mat.size(); ++row)
        {
            ret[row] += mat.get(row, column) / sum;
        }
        for (auto &v : ret)
        {
            v /= mat.size();
        }
    }
#endif

    double total = std::accumulate(ret.begin(), ret.end(), 0.0);
    for (auto &val : ret)
    {
        val /= total;
    }

    return ret;
}

auto getConsistency(const SquareMatrix &mat, std::vector<double> weight) -> double
{
    assert(weight.size() == mat.size() && "Incorrect matrix dimensions");

    std::vector<double> prod = mat.mul(weight);
    double ret = 0.0;

    for (size_t i{}; i < mat.size(); ++i)
    {
        ret += prod.at(i) / weight.at(i);
    }
    ret /= static_cast<double>(mat.size());

    return ret;
}

auto getConsistencyIndex(double consistency, size_t labels) -> double
{
    double labelsd{static_cast<double>(labels)};

    return (consistency - labelsd) / (labelsd - 1);
}

auto YAML_MapHasLabel(const YAML::Node &map, const std::string &label) -> bool
{
    assert(map.IsMap() && "Node is not a map!");
    assert(label.size() > 0 && "Cannot search for empty string!");

    return map[label].IsDefined();
}

auto YAML_MatrixFromSequence(const YAML::Node &seq) -> SquareMatrix
{
    std::vector<std::vector<double>> mat{};
    for (auto &&row : seq)
    {
        mat.push_back(row.as<std::vector<double>>());
    }

    return SquareMatrix{mat};
}

auto getDecisionVector(const YAML::Node &criteria) -> std::vector<double>
{
    assert(criteria.IsMap() && "Criteria must be a map!");
    assert(YAML_MapHasLabel(criteria, "comparisons") && "Criteria must have comparison matrix");

    SquareMatrix comparisonMat{YAML_MatrixFromSequence(criteria["comparisons"])};

    if (!YAML_MapHasLabel(criteria, "subcriteria"))
    {
        return getWeightVector(comparisonMat);
    }

    std::vector<double> comparisonWeightVector{getWeightVector(comparisonMat)};

    std::vector<std::vector<double>> childDecisionVector{};
    for (auto &&subcomparison : criteria["subcriteria"])
    {
        childDecisionVector.push_back(getDecisionVector(subcomparison));
    }

    std::vector<double> ret(childDecisionVector.at(0).size(), 0);

    for (size_t row{}; row < ret.size(); ++row)
    {
        for (size_t column{}; column < comparisonWeightVector.size(); ++column)
        {
            ret[row] += comparisonWeightVector[column] * childDecisionVector[column][row];
        }
    }

    double total = std::accumulate(ret.begin(), ret.end(), 0.0);
    for (auto &val : ret)
    {
        val /= total;
    }

    return ret;
}

auto getDecision(const YAML::Node &head) -> std::string
{
    assert(YAML_MapHasLabel(head, "criteria") && "Head node must have criteria!");
    std::vector<double> decisionVec{getDecisionVector(head["criteria"])};

    size_t maxIdx = 0;
    for (size_t idx{1}; idx < decisionVec.size(); ++idx)
    {
        if (decisionVec.at(idx) > decisionVec.at(maxIdx))
        {
            maxIdx = idx;
        }
    }

    assert(YAML_MapHasLabel(head, "labels") && "Head node must have labels!");
    auto labels = head["labels"].as<std::vector<std::string>>();
    assert(labels.size() >= decisionVec.size() && "Not enough labels!");

    return labels.at(maxIdx);
}
