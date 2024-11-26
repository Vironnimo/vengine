#include "utils.hpp"

namespace Vengine {

std::random_device Utils::rd;
std::mt19937 Utils::gen(Utils::rd());

float Utils::getRandomFloat(float min, float max) {
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(gen);
}

int Utils::getRandomInt(int min, int max) {
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(gen);
}

}  // namespace Vengine