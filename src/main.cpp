#include <iostream>
#include <fstream>
#include <cassert>
#include <filesystem>

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "No file provided\n";
        return 1;
    }

    std::filesystem::path filePath = argv[1];

    std::ifstream file(filePath);

    if (!file) {
        std::cerr << "Failed to open file\n";
        return 1;
    }

    std::cout << "Opened: "
              << std::filesystem::absolute(filePath) << "\n";

    return 0;

}