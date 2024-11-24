#pragma once

namespace Vengine {

// NOTE is this reliable at all?
template<typename T>
std::string getNameOfType(const T& type) {
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
        return typeName.substr(subStringIndex + 1); // Skip the ':' character
    }

    return typeName;
}

}
