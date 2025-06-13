#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "vengine/core/mesh.hpp"
#include "vengine/renderer/material.hpp"
#include "vengine/core/i_resource.hpp"

namespace Vengine {

class Model : public IResource {
   public:
    Model();
    ~Model() override;

    auto load(const std::string& fileName) -> bool override;
    auto finalizeOnMainThread() -> bool override;
    auto unload() -> bool override;
    [[nodiscard]] auto needsMainThreadInit() const -> bool override;

    [[nodiscard]] auto getMesh() const -> std::shared_ptr<Mesh>;
    [[nodiscard]] auto getDefaultMaterial() const -> std::shared_ptr<Material>;
    [[nodiscard]] auto getMaterialForSubmesh(const std::string& submeshName) const -> std::shared_ptr<Material>;

    auto setMesh(std::shared_ptr<Mesh> mesh) -> void;
    auto setDefaultMaterial(std::shared_ptr<Material> material) -> void;
    auto setMaterial(const std::string& submeshName, std::shared_ptr<Material> material) -> void;

   private:
    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<Material> m_defaultMaterial;
    std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;
};

}  // namespace Vengine