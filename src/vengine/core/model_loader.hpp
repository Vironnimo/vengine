#pragma once
#include <memory>
#include <string>
#include <filesystem>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "vengine/core/model.hpp"
#include "vengine/core/mesh_loader.hpp"
#include "vengine/core/shader.hpp"

namespace Vengine {

class ResourceManager;

class ModelLoader {
   public:
    ModelLoader(std::shared_ptr<MeshLoader> meshLoader, ResourceManager* resourceManager);
    ~ModelLoader();

    auto loadModel(const std::string& filename, std::shared_ptr<Shader> defaultShader = nullptr) -> std::shared_ptr<Model>;

   private:
    auto loadMaterialsFromMtl(const std::filesystem::path& mtlPath, std::shared_ptr<Shader> defaultShader)
        -> std::unordered_map<std::string, std::shared_ptr<Material>>;
    auto extractEmbeddedTexture(const aiTexture* texture, const std::string& texName) -> std::shared_ptr<Texture>;
    auto createDefaultMaterial(std::shared_ptr<Shader> defaultShader) -> std::shared_ptr<Material>;
    auto getTexturePath(const std::filesystem::path& mtlPath, const std::string& textureName) -> std::string;
    auto loadMaterialsFromAssimp(const std::string& filename, std::shared_ptr<Shader> defaultShader)
        -> std::unordered_map<std::string, std::shared_ptr<Material>>;

    std::shared_ptr<MeshLoader> m_meshLoader;
    ResourceManager* m_resourceManager;
};

}  // namespace Vengine