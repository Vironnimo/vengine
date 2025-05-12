#include "utils.hpp"
#include <fstream>

namespace Vengine {

std::random_device Utils::randomDevice;
bool Utils::genInitialized = false;
std::mt19937 Utils::gen;  

auto Utils::initGen() -> void {
    if (!genInitialized) {
        gen = std::mt19937(randomDevice());
        genInitialized = true;
    }
}

auto Utils::getRandomFloat(float min, float max) -> float {
    initGen();
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(gen);
}

auto Utils::getRandomInt(int min, int max) -> int {
    initGen();
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(gen);
}

auto Utils::readFile(const std::string& filePath) -> std::string {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return {};
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

}  // namespace Vengine