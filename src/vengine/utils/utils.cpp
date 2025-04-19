#include "utils.hpp"

namespace Vengine {

std::random_device Utils::randomDevice;
std::mt19937 Utils::gen(Utils::randomDevice());

auto Utils::getRandomFloat(float min, float max) -> float {
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(gen);
}

auto Utils::getRandomInt(int min, int max) -> int {
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(gen);
}

}  // namespace Vengine