#include <ahp.h>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include <repl.h>

int main(int argc, char *argv[])
{
    auto [nodeData, labels] = promptUserForHeadNode();
    std::shared_ptr<Node> root = std::make_shared<Node>(std::move(nodeData));

    std::shared_ptr<Node> currentNode = root;

    while (true)
    {
        currentNode = promptUserForNode(currentNode, labels, std::cin);
        
        if (!currentNode) {
            std::println("Navigation failed, resetting to root");
            currentNode = root;
        }
    }

    // assert(argc > 1 && "No file provided");

    YAML::Node config = YAML::LoadFile(/*argv[1]*/ "../college.yaml");

    YAML::Node criteria = config["criteria"]["subcriteria"];

    for (auto &&subcriteria : criteria)
    {

        std::cout << subcriteria["name"].as<std::string>() << "\n";
    }

    std::cout << config["criteria"]["comparisons"].IsDefined() << "\n";

    YAML::Node matNode = criteria[0]["subcriteria"][0]["comparisons"];
    std::vector<std::vector<double>> matVec{};

    for (auto &&row : matNode)
    {
        auto seq = row.as<std::vector<double>>();
        matVec.push_back(seq);
    }

    SquareMatrix mat{matVec};
    std::vector<double> weights = getWeightVector(mat);
    for (auto v : weights)
    {
        std::cout << v << ", ";
    }
    std::cout << "\n"
              << getConsistencyIndex(getConsistency(mat, weights), mat.size());

    std::cout << "\n"
              << YAML_MapHasLabel(criteria[0], "name") << ", " << YAML_MapHasLabel(criteria[0], "sigma") << "\n";

    std::vector<double> decision = getDecisionVector(config["criteria"]);

    for (auto &&entry : decision)
    {
        std::cout << entry << ", ";
    }

    std::cout << "\n"
              << getDecision(config) << "\n";

    // criteria[0]["subcriteria"][0]["comparisons"] -> Map<Seq<Scalar>>
    return 0;
}