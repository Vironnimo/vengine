#pragma once

#include <string>
#include <memory>
#include <filesystem>

#include "vengine/core/mesh.hpp"

namespace Vengine {

class MeshLoader {
   public:
    auto loadFromObj(const std::string& filename) -> std::shared_ptr<Mesh>;
    auto createPlane(float width = 1.0f, float height = 1.0f, int widthSegments = 1,
                     int heightSegments = 1) -> std::shared_ptr<Mesh>;

   private:
    auto getModelPath(const std::string& filename) -> std::filesystem::path;
};

}  // namespace Vengine