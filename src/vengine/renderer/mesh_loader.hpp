#pragma once

#include <string>
#include <memory>
#include <filesystem>

#include "vengine/renderer/mesh.hpp"

namespace Vengine {

class MeshLoader {
   public:
    auto loadFromObj(const std::string& filename) -> std::shared_ptr<Mesh>;

   private:
    auto getModelPath(const std::string& filename) -> std::filesystem::path;
};

}  // namespace Vengine