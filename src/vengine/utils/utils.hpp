#pragma once

#include <random>
#include <string>

namespace Vengine {

class Utils {
   public:
    // get classname as string
    template <typename T>
    static std::string getNameOfType(const T& type) {
        std::string typeName = typeid(type).name();

        // remove prefixes
        const std::string classPrefix = "class ";
        const std::string structPrefix = "struct ";

        if (typeName.find(classPrefix) == 0) {
            typeName = typeName.substr(classPrefix.length());
        } else if (typeName.find(structPrefix) == 0) {
            typeName = typeName.substr(structPrefix.length());
        }

        // remove namespace
        const auto subStringIndex = typeName.find_last_of(':');
        if (subStringIndex != std::string::npos) {
            return typeName.substr(subStringIndex + 1);
        }

        return typeName;
    }

    static float getRandomFloat(float min, float max);
    static int getRandomInt(int min, int max);

   private:
    static std::random_device rd;
    static std::mt19937 gen;
};

}  // namespace Vengine
