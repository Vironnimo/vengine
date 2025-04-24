#include "mesh_loader.hpp"
#include <cstddef>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <spdlog/spdlog.h>

namespace Vengine {

auto MeshLoader::loadFromObj(const std::string& filename) -> std::shared_ptr<Mesh> {
    auto modelPath = getModelPath(filename);

    // configure tinyobjloader
    tinyobj::ObjReaderConfig config;
    config.mtl_search_path = modelPath.parent_path().string();

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(modelPath.string(), config)) {
        if (!reader.Error().empty()) {
            spdlog::error("tinyobjreader error: {}", reader.Error());
        }
        spdlog::error("failed to load .obj file: {}", modelPath.string());
    }

    if (!reader.Warning().empty()) {
        spdlog::warn("tinyobjreader warning: {}", reader.Warning());
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();
    // const auto& materials = reader.GetMaterials();

    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    bool hasTexCoords = !attrib.texcoords.empty();
    int floatsPerVertex = hasTexCoords ? 5 : 3;

    // helper thingy to make sure we don't have duplicate vertices
    struct VertexKey {
        int v_idx, vt_idx, vn_idx;

        auto operator<(const VertexKey& other) const -> bool {
            return std::tie(v_idx, vt_idx, vn_idx) < std::tie(other.v_idx, other.vt_idx, other.vn_idx);
        }
    };

    std::map<VertexKey, uint32_t> uniqueVertices;

    for (const auto& shape : shapes) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            size_t fv = shape.mesh.num_face_vertices[f];

            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

                VertexKey key{idx.vertex_index, idx.texcoord_index, idx.normal_index};

                if (uniqueVertices.count(key) == 0) {
                    uniqueVertices[key] = static_cast<uint32_t>(vertices.size() / static_cast<size_t>(floatsPerVertex));

                    // position
                    vertices.push_back(attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 0]);
                    vertices.push_back(attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 1]);
                    vertices.push_back(attrib.vertices[3 * static_cast<size_t>(idx.vertex_index) + 2]);

                    // texture stuff if available
                    if (hasTexCoords && idx.texcoord_index >= 0) {
                        vertices.push_back(attrib.texcoords[2 * static_cast<size_t>(idx.texcoord_index) + 0]);
                        vertices.push_back(attrib.texcoords[2 * static_cast<size_t>(idx.texcoord_index) + 1]);
                    } else if (hasTexCoords) {
                        // add dummy tex coords 
                        vertices.push_back(0.0f);
                        vertices.push_back(0.0f);
                    }
                }

                indices.push_back(uniqueVertices[key]);
            }
            index_offset += static_cast<size_t>(fv);
        }
    }

    auto mesh = std::make_shared<Mesh>(vertices, indices);
    // mesh->m_hasTexCoords = hasTexCoords;

    return mesh;
}

auto MeshLoader::createPlane(float width, float height, int widthSegments, int heightSegments) -> std::shared_ptr<Mesh> {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    const int floatsPerVertex = 5; // 3 position + 2 texCoords

    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;

    float segmentWidth = width / static_cast<float>(widthSegments);
    float segmentHeight = height / static_cast<float>(heightSegments);

    // Generate vertices (Position + TexCoords)
    for (int iy = 0; iy <= heightSegments; ++iy) {
        float y = static_cast<float>(iy) * segmentHeight - halfHeight; // This is actually z in world space for XZ plane
        float v = static_cast<float>(iy) / static_cast<float>(heightSegments);

        for (int ix = 0; ix <= widthSegments; ++ix) {
            float x = static_cast<float>(ix) * segmentWidth - halfWidth;
            float u = static_cast<float>(ix) / static_cast<float>(widthSegments);

            // Position (X, Y, Z) - Plane on XZ plane, Y=0
            vertices.push_back(x);
            vertices.push_back(0.0f);
            vertices.push_back(y); // Use y loop variable for Z coordinate

            // Texture Coordinates (U, V)
            vertices.push_back(u);
            vertices.push_back(v);

            // Note: Normals (0, 1, 0) are omitted here to match the apparent format
            // from loadFromObj and the Mesh constructor. If your shaders require
            // normals, you'll need to adjust the Mesh/VertexBuffer setup to handle them.
        }
    }

    // Generate indices
    for (uint32_t iy = 0; iy < static_cast<uint32_t>(heightSegments); ++iy) {
        for (uint32_t ix = 0; ix < static_cast<uint32_t>(widthSegments); ++ix) {
            // Calculate indices for the four corners of the quad
            uint32_t rowStride = static_cast<uint32_t>(widthSegments) + 1;
            uint32_t topLeft = iy * rowStride + ix;
            uint32_t topRight = topLeft + 1;
            uint32_t bottomLeft = (iy + 1) * rowStride + ix;
            uint32_t bottomRight = bottomLeft + 1;

            // First triangle (top-left, bottom-left, top-right)
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // Second triangle (top-right, bottom-left, bottom-right)
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    spdlog::debug("Created plane mesh via MeshLoader: {} floats ({} vertices), {} indices", vertices.size(), vertices.size() / floatsPerVertex, indices.size());

    // Create and return the mesh object using the appropriate constructor
    return std::make_shared<Mesh>(vertices, indices);
}

auto MeshLoader::getModelPath(const std::string& filename) -> std::filesystem::path {
    return std::filesystem::path("resources/models") / filename;
}

}  // namespace Vengine