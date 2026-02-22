#include <iostream>
#include <fstream>
#include <cassert>
#include <filesystem>
#include <vector>
#include <ahp.h>

#include <yaml-cpp/yaml.h>


int main(int argc, char* argv[]) {

    assert (argc > 1 && "No file provided");

    YAML::Node config = YAML::LoadFile(argv[1]);

    YAML::Node criteria = config["criteria"];

    for (size_t i = 0; i < criteria.size(); ++i) {

        std::cout << criteria[i]["name"].as<std::string>() << "\n";
    }
    
    std::cout << criteria[0]["name"].Type() << "\n";

    YAML::Node matNode = criteria[0]["subcriteria"][0]["comparisons"];
    std::vector<std::vector<double>> matVec{};

    for (size_t i = 0; i < matNode.size(); ++i) {
        std::vector seq = matNode[i].as<std::vector<double>>();
        matVec.push_back(seq);
    }

    SquareMatrix mat{matVec};
    std::vector<double> weights = getWeightVector(mat);
    for (auto v : weights) {
        std::cout << v << ", ";
    }
    std::cout << "\n" << getConsistencyIndex(getConsistency(mat, weights), mat.size());
    

//criteria[0]["subcriteria"][0]["comparisons"] -> Map<Seq<Scalar>>
    return 0;

}