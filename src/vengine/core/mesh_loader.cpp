#include "mesh_loader.hpp"
#include <cstddef>

#include <assimp/Importer.hpp>
#include <cstdint>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <spdlog/spdlog.h>
#include "vengine/renderer/vertex_layout.hpp"

namespace Vengine {

MeshLoader::MeshLoader() {
    spdlog::debug("Constructor MeshLoader");
}

MeshLoader::~MeshLoader() {
    spdlog::debug("Destructor MeshLoader");
}

auto MeshLoader::loadModel(const std::string& filename) -> std::shared_ptr<Mesh> {
    auto modelPath = getModelPath(filename);
    
    Assimp::Importer importer;
    
    const aiScene* scene = importer.ReadFile(
        modelPath.string(),
        aiProcess_Triangulate |           // ensure triangles
        aiProcess_GenSmoothNormals |      // generate normals if not present
        aiProcess_FlipUVs |               // flip tex coords (opengl needs this)
        aiProcess_CalcTangentSpace |      // ??
        aiProcess_JoinIdenticalVertices |
        aiProcess_ValidateDataStructure  
    );
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        spdlog::error("Assimp error: {}", importer.GetErrorString());
        return nullptr;
    }
    
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    std::vector<Submesh> submeshes;
    
    bool hasTexCoords = false;
    bool hasNormals = true;  // always true i guess
    
    size_t totalVertexCount = 0;
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];
        size_t submeshStartIndex = indices.size();
        
        // get material name for mesh
        std::string materialName;
        if (mesh->mMaterialIndex < scene->mNumMaterials) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            aiString name;
            if (material->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
                materialName = name.C_Str();
            } else {
                materialName = "material_" + std::to_string(mesh->mMaterialIndex);
            }
            
            m_materialCache[std::to_string(mesh->mMaterialIndex)] = materialName;
        }
        
        size_t vertexStartIndex = vertices.size() / 8;  // 8 = 3 pos + 2 tex + 3 norm
        hasTexCoords = mesh->mTextureCoords[0] != nullptr;
        
        // process vertices
        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
            // position
            vertices.push_back(mesh->mVertices[j].x);
            vertices.push_back(mesh->mVertices[j].y);
            vertices.push_back(mesh->mVertices[j].z);
            
            // texture coordinates 
            if (hasTexCoords) {
                vertices.push_back(mesh->mTextureCoords[0][j].x);
                vertices.push_back(mesh->mTextureCoords[0][j].y);
            } else {
                // defaults
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }
            
            // normals (always present due to aiProcess_GenSmoothNormals)
            vertices.push_back(mesh->mNormals[j].x);
            vertices.push_back(mesh->mNormals[j].y);
            vertices.push_back(mesh->mNormals[j].z);
        }
        
        totalVertexCount += mesh->mNumVertices;
        
        // indices
        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
            aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++) {
                indices.push_back(face.mIndices[k] + static_cast<uint32_t>(vertexStartIndex));
            }
        }
        
        Submesh submesh;
        submesh.indexOffset = static_cast<uint32_t>(submeshStartIndex);
        submesh.indexCount = static_cast<uint32_t>(indices.size() - submeshStartIndex);
        submesh.materialName = materialName;
        submeshes.push_back(submesh);
        
        // spdlog::debug("Processed mesh {} with {} vertices, {} indices, material: {}", 
                    //  i, mesh->mNumVertices, mesh->mNumFaces * 3, materialName);
    }
    
    // Validate vertex data consistency
    VertexLayout layout;
    layout.hasPosition = true;
    layout.hasTexCoords = true; 
    layout.hasNormals = hasNormals;

    int floatsPerVertex = layout.calculateStride() / static_cast<int>(sizeof(float));
    size_t expectedFloats = totalVertexCount * static_cast<size_t>(floatsPerVertex);
    
    if (vertices.size() != expectedFloats) {
        spdlog::warn("Vertex data inconsistency detected. Expected {} floats but got {}. Fixing...", 
                    expectedFloats, vertices.size());
        
        // fix vertex data if necessary 
        if (vertices.size() > expectedFloats) {
            vertices.resize(expectedFloats);
        } else if (vertices.size() < expectedFloats) {
            vertices.resize(expectedFloats, 0.0f);
        }
    }
    
    spdlog::debug("Created mesh with {} vertices, {} indices, {} submeshes", 
                 vertices.size() / static_cast<size_t>(floatsPerVertex), indices.size(), submeshes.size());
    
    auto result = std::make_shared<Mesh>(vertices, indices, layout);
    for (const auto& submesh : submeshes) {
        result->addSubmesh(submesh);
    }
    
    return result;
}

auto MeshLoader::createPlane(float width, float height, int widthSegments, int heightSegments) -> std::shared_ptr<Mesh> {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;

    float segmentWidth = width / static_cast<float>(widthSegments);
    float segmentHeight = height / static_cast<float>(heightSegments);

    // generate vertices
    for (int iy = 0; iy <= heightSegments; ++iy) {
        float y = static_cast<float>(iy) * segmentHeight - halfHeight;
        float v = static_cast<float>(iy) / static_cast<float>(heightSegments);

        for (int ix = 0; ix <= widthSegments; ++ix) {
            float x = static_cast<float>(ix) * segmentWidth - halfWidth;
            float u = static_cast<float>(ix) / static_cast<float>(widthSegments);

            // position (x, y, z)
            vertices.push_back(x);
            vertices.push_back(0.0f);
            vertices.push_back(y);

            // texture Coordinates (u, v)
            vertices.push_back(u);
            vertices.push_back(v);
        }
    }

    // generate indices
    for (uint32_t iy = 0; iy < static_cast<uint32_t>(heightSegments); ++iy) {
        for (uint32_t ix = 0; ix < static_cast<uint32_t>(widthSegments); ++ix) {
            // calculate indices
            uint32_t rowStride = static_cast<uint32_t>(widthSegments) + 1;
            uint32_t topLeft = iy * rowStride + ix;
            uint32_t topRight = topLeft + 1;
            uint32_t bottomLeft = (iy + 1) * rowStride + ix;
            uint32_t bottomRight = bottomLeft + 1;

            // first triangle
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // second triangle
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    return std::make_shared<Mesh>(vertices, indices, VertexLayout{true, true, false});
}

auto MeshLoader::getModelPath(const std::string& filename) -> std::filesystem::path {
    return std::filesystem::path("resources/models") / filename;
}

}  // namespace Vengine