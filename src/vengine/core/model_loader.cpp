#include "model_loader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <algorithm>

#include <fstream>
#include <sstream>
#include <spdlog/spdlog.h>
#include "vengine/core/resource_manager.hpp"
#include "vengine/core/resources.hpp"

namespace Vengine {

ModelLoader::ModelLoader(std::shared_ptr<MeshLoader> meshLoader, ResourceManager* resourceManager)
    : m_meshLoader(std::move(meshLoader)), m_resourceManager(resourceManager) {
    spdlog::debug("Constructor ModelLoader");
}

ModelLoader::~ModelLoader() {
    spdlog::debug("Destructor ModelLoader");
}

auto ModelLoader::loadModel(const std::string& filename, std::shared_ptr<Shader> defaultShader) -> std::shared_ptr<Model> {
    auto model = std::make_shared<Model>();
    
    // Load the mesh and get the scene
    auto mesh = m_meshLoader->loadModel(filename);
    if (!mesh) {
        spdlog::error("Failed to load mesh for model: {}", filename);
        return nullptr;
    }
    
    // Set the mesh to the model
    model->setMesh(mesh);
    
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;
    
    // First try to load materials from Assimp scene
    materials = loadMaterialsFromAssimp(filename, defaultShader);
    
    // If no materials were loaded from Assimp and it's an OBJ file, try MTL as fallback
    if (materials.empty()) {
        auto modelPath = m_meshLoader->getModelPath(filename);
        auto extension = modelPath.extension().string();
        if (extension == ".obj" || extension == ".OBJ") {
            auto mtlPath = modelPath;
            mtlPath.replace_extension(".mtl");
            
            // Check if MTL file exists
            if (std::filesystem::exists(mtlPath)) {
                materials = loadMaterialsFromMtl(mtlPath, defaultShader);
            }
        }
    }
    
    // If we have materials, associate them with the model
    if (!materials.empty()) {
        // Set the first material as default
        auto firstMaterial = materials.begin();
        model->setDefaultMaterial(firstMaterial->second);
        
        // Set all materials
        for (const auto& [name, material] : materials) {
            model->setMaterial(name, material);
        }
    } else {
        // Create a default material
        model->setDefaultMaterial(createDefaultMaterial(defaultShader));
    }
    
    return model;
}

auto ModelLoader::loadMaterialsFromAssimp(const std::string& filename, 
                                         std::shared_ptr<Shader> defaultShader) 
    -> std::unordered_map<std::string, std::shared_ptr<Material>> {
    
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;
    
    // Get the model path
    auto modelPath = m_meshLoader->getModelPath(filename);
    auto modelDir = modelPath.parent_path();
    
    // Load the scene again with Assimp to access materials
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        modelPath.string(),
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices
    );
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        spdlog::error("Assimp error in material loading: {}", importer.GetErrorString());
        return materials;
    }
    
    // Log embedded texture information
    if (scene->mNumTextures > 0) {
        spdlog::info("Model has {} embedded textures", scene->mNumTextures);
        for (unsigned int i = 0; i < scene->mNumTextures; i++) {
            const aiTexture* texture = scene->mTextures[i];
            spdlog::debug("Embedded texture {}: {}x{}, format: {}", 
                i, texture->mWidth, texture->mHeight, texture->achFormatHint);
        }
    }
    
    // Process all materials
    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* aiMat = scene->mMaterials[i];
        aiString name;
        aiMat->Get(AI_MATKEY_NAME, name);
        
        std::string matName = name.length > 0 ? name.C_Str() : "material_" + std::to_string(i);
        
        auto material = std::make_shared<Material>(defaultShader);
        
        // Extract diffuse color
        aiColor3D diffuseColor(0.7f, 0.7f, 0.7f);
        if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS) {
            material->setVec3("uDiffuse", glm::vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b));
            material->setVec4("uColor", glm::vec4(diffuseColor.r, diffuseColor.g, diffuseColor.b, 1.0f));
        }
        
        // Extract ambient color
        aiColor3D ambientColor(1.0f, 1.0f, 1.0f);
        if (aiMat->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor) == AI_SUCCESS) {
            material->setVec3("uAmbient", glm::vec3(ambientColor.r, ambientColor.g, ambientColor.b));
        }
        
        // Extract specular color
        aiColor3D specularColor(0.5f, 0.5f, 0.5f);
        if (aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == AI_SUCCESS) {
            material->setVec3("uSpecular", glm::vec3(specularColor.r, specularColor.g, specularColor.b));
        }
        
        // Extract shininess
        float shininess = 32.0f;
        if (aiMat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
            material->setFloat("uShininess", shininess);
        }
        
        // Check for diffuse textures
        aiString texturePath;
        if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
            std::string texName = matName + "_diffuse";
            bool textureLoaded = false;
            
            // Check if this is an embedded texture (path starts with '*')
            if (texturePath.length > 0 && texturePath.C_Str()[0] == '*') {
                spdlog::info("Found embedded texture reference: {}", texturePath.C_Str());
                
                // Extract texture index from the path (remove the '*')
                int textureIndex = std::stoi(texturePath.C_Str() + 1);
                
                if (textureIndex >= 0 && textureIndex < static_cast<int>(scene->mNumTextures)) {
                    // Get the embedded texture
                    const aiTexture* texture = scene->mTextures[textureIndex];
                    
                    // Extract and create texture
                    auto extractedTexture = extractEmbeddedTexture(texture, texName);
                    if (extractedTexture) {
                        material->setBool("uUseTexture", true);
                        material->setTexture("uTexture", extractedTexture);
                        textureLoaded = true;
                        spdlog::info("Successfully loaded embedded texture for material: {}", matName);
                    }
                } else {
                    spdlog::error("Invalid embedded texture index: {}", textureIndex);
                }
            }
            
            // If no embedded texture was loaded, try loading from file
            if (!textureLoaded) {
                std::filesystem::path texFilePath = modelDir / texturePath.C_Str();
                
                // Normalize path for Windows
                std::string fullTexPath = texFilePath.string();
                std::replace(fullTexPath.begin(), fullTexPath.end(), '\\', '/');
                
                spdlog::debug("Loading texture from path: {}", fullTexPath);
                
                // Try to load the texture
                if (std::filesystem::exists(texFilePath)) {
                    if (!m_resourceManager->isLoaded(texName)) {
                        m_resourceManager->load<Texture>(texName, fullTexPath);
                    }
                    
                    auto texture = m_resourceManager->get<Texture>(texName);
                    if (texture) {
                        material->setBool("uUseTexture", true);
                        material->setTexture("uTexture", texture);
                        spdlog::debug("Successfully loaded texture for material: {}", matName);
                    } else {
                        material->setBool("uUseTexture", false);
                        spdlog::error("Failed to load texture: {}", fullTexPath);
                    }
                } else {
                    spdlog::error("Texture file does not exist: {}", fullTexPath);
                    material->setBool("uUseTexture", false);
                }
            }
        } else {
            material->setBool("uUseTexture", false);
        }
        
        // Add the material
        materials[matName] = material;
    }
    
    return materials;
}

auto ModelLoader::extractEmbeddedTexture(const aiTexture* texture, const std::string& texName) 
    -> std::shared_ptr<Texture> {
    
    if (!texture) {
        spdlog::error("Null texture pointer");
        return nullptr;
    }
    
    // Create shared pointer for raw image data
    std::shared_ptr<Texture> resultTexture = nullptr;
    
    // Check if the texture is already loaded
    if (m_resourceManager->isLoaded(texName)) {
        return m_resourceManager->get<Texture>(texName);
    }
    
    // Check if the texture is compressed (stored in a common format)
    if (texture->mHeight == 0) {
        spdlog::info("Processing compressed embedded texture ({} format), size: {} bytes", 
                   texture->achFormatHint, texture->mWidth);
        
        // mWidth contains the size in bytes for compressed textures
        int width, height, channels;
        
        // Decode the compressed texture data
        unsigned char* pixels = stbi_load_from_memory(
            reinterpret_cast<const stbi_uc*>(texture->pcData),
            texture->mWidth, // Size in bytes
            &width, &height, &channels, 4 // Force RGBA
        );
        
        if (pixels) {
            spdlog::info("Decoded embedded texture: {}x{}, channels: {}", width, height, channels);
            
            // Create a texture resource from the decoded data
            resultTexture = std::make_shared<Texture>();
            resultTexture->setRawData(pixels, width, height, 4);
            resultTexture->setName(texName);
            
            // Register in resource manager
            m_resourceManager->add<Texture>(texName, resultTexture);
            
            // Clean up stbi allocated data after texture is created
            stbi_image_free(pixels);
        } else {
            spdlog::error("Failed to decode embedded texture: {}", stbi_failure_reason());
        }
    } 
    else {
        // Uncompressed texture with raw pixel data
        spdlog::info("Processing uncompressed embedded texture: {}x{}", 
                   texture->mWidth, texture->mHeight);
        
        // Create texture from raw data
        unsigned char* pixels = new unsigned char[texture->mWidth * texture->mHeight * 4];
        
        // Copy pixel data (assuming RGBA for now)
        for (unsigned int y = 0; y < texture->mHeight; ++y) {
            for (unsigned int x = 0; x < texture->mWidth; ++x) {
                aiTexel& texel = texture->pcData[y * texture->mWidth + x];
                unsigned char* pixel = &pixels[(y * texture->mWidth + x) * 4];
                
                pixel[0] = texel.r;
                pixel[1] = texel.g;
                pixel[2] = texel.b;
                pixel[3] = texel.a;
            }
        }
        
        // Create a texture resource from the raw data
        resultTexture = std::make_shared<Texture>();
        resultTexture->setRawData(pixels, texture->mWidth, texture->mHeight, 4);
        resultTexture->setName(texName);
        
        // Register in resource manager
        m_resourceManager->add<Texture>(texName, resultTexture);
        
        // Clean up allocated data after texture is created
        delete[] pixels;
    }
    
    return resultTexture;
}

auto ModelLoader::loadMaterialsFromMtl(const std::filesystem::path& mtlPath, 
                                      std::shared_ptr<Shader> defaultShader) 
    -> std::unordered_map<std::string, std::shared_ptr<Material>> {
    
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;
    std::string currentMaterial;
    
    std::ifstream file(mtlPath);
    if (!file.is_open()) {
        spdlog::error("Failed to open MTL file: {}", mtlPath.string());
        return materials;
    }
    
    std::string line;
    glm::vec3 ambient(0.1f);
    glm::vec3 diffuse(0.7f); // Default gray
    glm::vec3 specular(0.5f);
    float shininess = 32.0f;
    std::string diffuseTexture;
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        
        if (!(iss >> type)) {
            continue; // Empty line
        }
        
        if (type == "newmtl") {
            // Save the previous material before starting a new one
            if (!currentMaterial.empty()) {
                auto material = std::make_shared<Material>(defaultShader);
                
                // Set material properties
                material->setVec3("uAmbient", ambient);
                material->setVec3("uDiffuse", diffuse);
                material->setVec3("uSpecular", specular);
                material->setFloat("uShininess", shininess);
                
                // Handle texture if present
                if (!diffuseTexture.empty()) {
                    std::string textureName = currentMaterial + "_diffuse";
                    std::string texturePath = getTexturePath(mtlPath, diffuseTexture);
                    
                    auto texture = m_resourceManager->get<Texture>(textureName);
                    if (!texture) {
                        // Load the texture if not already loaded
                        m_resourceManager->load<Texture>(textureName, texturePath);
                        texture = m_resourceManager->get<Texture>(textureName);
                    }
                    
                    if (texture) {
                        material->setBool("uUseTexture", true);
                        material->setTexture("uTexture", texture);
                    } else {
                        // Fallback to diffuse color
                        material->setBool("uUseTexture", false);
                        material->setVec4("uColor", glm::vec4(diffuse, 1.0f));
                    }
                } else {
                    // No texture, use diffuse color
                    material->setBool("uUseTexture", false);
                    material->setVec4("uColor", glm::vec4(diffuse, 1.0f));
                }
                
                materials[currentMaterial] = material;
                
                // Reset for next material
                ambient = glm::vec3(0.1f);
                diffuse = glm::vec3(0.7f);
                specular = glm::vec3(0.5f);
                shininess = 32.0f;
                diffuseTexture.clear();
            }
            
            iss >> currentMaterial;
        } else if (type == "Ka") { // Ambient
            iss >> ambient.r >> ambient.g >> ambient.b;
        } else if (type == "Kd") { // Diffuse
            iss >> diffuse.r >> diffuse.g >> diffuse.b;
        } else if (type == "Ks") { // Specular
            iss >> specular.r >> specular.g >> specular.b;
        } else if (type == "Ns") { // Shininess
            iss >> shininess;
        } else if (type == "map_Kd") { // Diffuse texture
            iss >> diffuseTexture;
        }
    }
    
    // Handle the last material
    if (!currentMaterial.empty()) {
        auto material = std::make_shared<Material>(defaultShader);
        
        material->setVec3("uAmbient", ambient);
        material->setVec3("uDiffuse", diffuse);
        material->setVec3("uSpecular", specular);
        material->setFloat("uShininess", shininess);
        
        if (!diffuseTexture.empty()) {
            std::string textureName = currentMaterial + "_diffuse";
            std::string texturePath = getTexturePath(mtlPath, diffuseTexture);
            
            auto texture = m_resourceManager->get<Texture>(textureName);
            if (!texture) {
                m_resourceManager->load<Texture>(textureName, texturePath);
                texture = m_resourceManager->get<Texture>(textureName);
            }
            
            if (texture) {
                material->setBool("uUseTexture", true);
                material->setTexture("uTexture", texture);
            } else {
                material->setBool("uUseTexture", false);
                material->setVec4("uColor", glm::vec4(diffuse, 1.0f));
            }
        } else {
            material->setBool("uUseTexture", false);
            material->setVec4("uColor", glm::vec4(diffuse, 1.0f));
        }
        
        materials[currentMaterial] = material;
    }
    
    return materials;
}

auto ModelLoader::createDefaultMaterial(std::shared_ptr<Shader> defaultShader) -> std::shared_ptr<Material> {
    auto material = std::make_shared<Material>(defaultShader);
    material->setBool("uUseTexture", false);
    material->setVec4("uColor", glm::vec4(0.7f, 0.7f, 0.7f, 1.0f)); // Default gray
    return material;
}

auto ModelLoader::getTexturePath(const std::filesystem::path& mtlPath, const std::string& textureName) -> std::string {
    // Try to find the texture in multiple possible locations
    auto texturePath = mtlPath.parent_path() / textureName;
    if (std::filesystem::exists(texturePath)) {
        return texturePath.string();
    }
    
    // Try the textures subfolder
    texturePath = mtlPath.parent_path() / "textures" / textureName;
    if (std::filesystem::exists(texturePath)) {
        return texturePath.string();
    }
    
    // Just return the original name and let the resource manager handle it
    return textureName;
}

}  // namespace Vengine