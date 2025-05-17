#include "vengine/core/model.hpp"
#include <spdlog/spdlog.h>

namespace Vengine {

Model::Model() {
    spdlog::debug("Constructor Model");
}

Model::~Model() {
    spdlog::debug("Destructor Model");
    unload();
}

auto Model::load(const std::string& fileName) -> bool {
    // This is intentionally minimal as actual loading will be handled by ModelLoader
    return true;
}

auto Model::finalizeOnMainThread() -> bool {
    if (m_mesh) {
        return m_mesh->finalizeOnMainThread();
    }
    return false;
}

auto Model::unload() -> bool {
    if (m_mesh) {
        m_mesh->unload();
    }
    m_defaultMaterial.reset();
    m_materials.clear();
    return true;
}

auto Model::getMaterialForSubmesh(const std::string& submeshName) const -> std::shared_ptr<Material> {
    auto it = m_materials.find(submeshName);
    if (it != m_materials.end()) {
        return it->second;
    }
    return m_defaultMaterial;
}

auto Model::setMaterial(const std::string& submeshName, std::shared_ptr<Material> material) -> void {
    m_materials[submeshName] = std::move(material);
}

}  // namespace Vengine