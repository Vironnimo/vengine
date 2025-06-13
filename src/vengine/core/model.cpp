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
    // m_mesh->load(fileName);
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

auto Model::needsMainThreadInit() const -> bool {
    return m_mesh && m_mesh->needsMainThreadInit();
}

auto Model::getMesh() const -> std::shared_ptr<Mesh> {
    return m_mesh;
}

auto Model::getDefaultMaterial() const -> std::shared_ptr<Material> {
    return m_defaultMaterial;
}

auto Model::getMaterialForSubmesh(const std::string& submeshName) const -> std::shared_ptr<Material> {
    auto it = m_materials.find(submeshName);
    if (it != m_materials.end()) {
        return it->second;
    }
    return m_defaultMaterial;
}

auto Model::setMesh(std::shared_ptr<Mesh> mesh) -> void {
    m_mesh = std::move(mesh);
}

auto Model::setDefaultMaterial(std::shared_ptr<Material> material) -> void {
    m_defaultMaterial = std::move(material);
}

auto Model::setMaterial(const std::string& submeshName, std::shared_ptr<Material> material) -> void {
    m_materials[submeshName] = std::move(material);
}

}  // namespace Vengine