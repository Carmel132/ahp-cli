// #include <cmath>
#include <matrix.h>
#include <numeric>
#include <string>

#include <yaml-cpp/yaml.h>

#define COMPUTE_GEOM_MEAN_WEIGHT_VECTOR

auto getWeightVector(const SquareMatrix &mat) -> std::vector<double>;

auto getConsistency(const SquareMatrix &mat, std::vector<double> weight) -> double;

auto getConsistencyIndex(double consistency, size_t labels) -> double;

auto YAML_MapHasLabel(const YAML::Node &map, const std::string &label) -> bool;

auto YAML_MatrixFromSequence(const YAML::Node &seq) -> SquareMatrix;

auto getDecisionVector(const YAML::Node &criteria) -> std::vector<double>;

auto getDecision(const YAML::Node &head) -> std::string;