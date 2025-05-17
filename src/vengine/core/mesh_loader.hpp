#pragma once

#include <string>
#include <memory>
#include <filesystem>
#include <unordered_map>

#include "vengine/core/mesh.hpp"

namespace Vengine {

class MeshLoader {
   public:
    MeshLoader();
    ~MeshLoader();
    auto loadModel(const std::string& filename) -> std::shared_ptr<Mesh>;
    auto createPlane(float width = 100.0f, float height = 100.0f, int widthSegments = 1,
                     int heightSegments = 1) -> std::shared_ptr<Mesh>;
    auto getModelPath(const std::string& filename) -> std::filesystem::path;

   private:

    std::unordered_map<std::string, std::string> m_materialCache;
};

}  // namespace Vengine