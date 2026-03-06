#pragma once

#include <charconv>
#include <concepts>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <print>
#include <ranges>
#include <string>
#include <vector>

#include <matrix.h>

auto tryParseInt(std::string_view str) -> std::optional<int>;

auto promptUserForInt(const std::string &message) -> int;

template <std::ranges::input_range R>
auto promptUserWithOption(const R &options) -> int
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

auto promptUserForBool(const std::string &message) -> bool;

auto promptUserForString(const std::string &message) -> std::string;

template <std::ranges::input_range R>
auto promptUserForWeightMatrix(const R &labels) -> SquareMatrix
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

struct Node
{
    std::string name_;
    SquareMatrix matrix_;
    std::optional<std::vector<Node>> subcriteria_;
    std::optional<std::shared_ptr<Node>> enclosingCriteria_;

    auto collectSubcriteriaNames();
};

auto promptUserForHeadNode() -> std::pair<Node, std::vector<std::string>>;

template <typename T>
using NodePromptFunction = std::function<T(Node &, const std::vector<std::string> &)>;

struct NodePromptOption
{
    NodePromptFunction<std::string> getName;
    NodePromptFunction<Node> promptAction;
    NodePromptFunction<bool> isAvailable;
};

auto promptUserForNode(Node &currentNode, const std::vector<std::string> &labels) -> Node;
/*
1. Ask for labels and top level objective
2. Print the top level node and present the following options
 - Change name
 - Change matrix
 - [Add/See] subcriteria
 - Move up (if present)
*/
struct REPL
{
};