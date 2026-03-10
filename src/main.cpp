#include <ahp.h>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include <repl.h>

int main(int argc, char *argv[])
{
    std::shared_ptr<Node> root;
    std::vector<std::string> labelVector;
    if (argc > 1) {
        YAML::Node config = YAML::LoadFile(argv[1]);

        auto [nodeData, labels] = fromYAMLNode(config);
    
        root = nodeData;
        labelVector = labels;
    }
    else {
        auto [nodeData, labels] = promptUserForHeadNode();
        root = std::make_shared<Node>(std::move(nodeData));
        labelVector = labels;
    }

    std::shared_ptr<Node> currentNode = root;

    while (true)
    {
        currentNode = promptUserForNode(currentNode, labelVector, std::cin);

        if (!currentNode)
        {
            std::println("Navigation failed, resetting to root");
            currentNode = root;
        }
    }
    return 0;
}