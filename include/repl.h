#pragma once

#include <charconv>
#include <concepts>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <print>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

#include <yaml-cpp/yaml.h>

#include <ahp.h>
#include <matrix.h>

auto tryParseInt(std::string_view str) -> std::optional<int>;

auto promptUserForInt(const std::string &message, std::istream &input = std::cin) -> int;

template <std::ranges::input_range R>
auto promptUserWithOption(const R &options, std::istream &input = std::cin) -> int
{
    assert(!std::ranges::empty(options) && "Must have at least one option");

    size_t idx = 1;
    for (const auto &option : options)
    {
        std::println("[{}] {}", idx++, option);
    }

    std::print("\n?> ");
    std::string response;
    std::getline(std::cin, response);
    std::optional<int> numResponse = tryParseInt(response);
    while (!numResponse.has_value() || numResponse.value() - 1 >= std::ranges::distance(options))
    {
        std::print("Invalid input!\n?> ");
        std::getline(std::cin, response);
        numResponse = tryParseInt(response);
    }
    return numResponse.value() - 1;
}

auto promptUserForBool(const std::string &message, std::istream &input = std::cin) -> bool;

auto promptUserForString(const std::string &message, std::istream &input = std::cin) -> std::string;

template <std::ranges::input_range R>
auto promptUserForWeightMatrix(const R &labels, std::istream &input = std::cin) -> SquareMatrix
{
    assert(!std::ranges::empty(labels) && "Must provide labels!");

    const auto n = std::ranges::size(labels);
    std::vector<std::vector<double>> ret;
    ret.reserve(n);

    for (size_t row = 0; row < n; ++row)
    {
        std::vector<double> temp(n);
        ret.emplace_back(std::move(temp));

        for (size_t column = 0; column < n; ++column)
        {
            if (row == column)
            {
                ret[row][column] = 1;
            }
            else if (row < column)
            {
                ret[row][column] = promptUserForInt(std::format("How much more do you like {} over {}", labels[row], labels[column]));
            }
            else
            {
                ret[row][column] = 1 / ret[column][row];
            }
        }
    }

    return {ret};
}

class Node : public std::enable_shared_from_this<Node>
{
public:
    std::string name_;
    SquareMatrix matrix_;
    std::optional<std::vector<std::shared_ptr<Node>>> subcriteria_;
    std::optional<std::weak_ptr<Node>> enclosingCriteria_;

    Node(const std::string &name, const SquareMatrix &matrix, std::optional<std::vector<std::shared_ptr<Node>>> subcriteria = std::nullopt, std::optional<std::weak_ptr<Node>> enclosingCriteria = std::nullopt) : name_{name}, matrix_{matrix}, subcriteria_{std::move(subcriteria)}, enclosingCriteria_{std::move(enclosingCriteria)} {}

    auto collectSubcriteriaNames();
};
using Node_ptr = std::shared_ptr<Node>;

void invalidateMatrix(Node_ptr &node);

auto getRootNode(const Node_ptr &node) -> Node_ptr;

auto addYAMLSubcriteria(YAML::Node &head, const Node_ptr &node, const std::vector<std::string> &labels) -> YAML::Node;

auto convertToYAMLNode(const Node_ptr &node, const std::vector<std::string> &labels) -> YAML::Node;

auto promptUserForHeadNode(std::istream &input = std::cin) -> std::pair<Node, std::vector<std::string>>;

template <typename T>
using NodePromptFunction = std::function<T(Node_ptr &, const std::vector<std::string> &)>;

struct NodePromptOption
{
    NodePromptFunction<std::string> getName;
    NodePromptFunction<Node_ptr> promptAction;
    NodePromptFunction<bool> isAvailable;
};

auto promptUserForNode(Node_ptr currentNode, const std::vector<std::string> &labels, std::istream &input = std::cin) -> Node_ptr;
/*
1. Ask for labels and top level objective
2. Print the top level node and present the following options
 - Change name
 - Change matrix
 - Add subcriteria*
 - Remove subcriteria* (if present)
 - See subcriteria (if present)
 - Move up (if present)
 - Export
 - Run

 * = invalidates weight matrix
*/