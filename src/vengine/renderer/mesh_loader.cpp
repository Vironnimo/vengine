#include "mesh_loader.hpp"

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
            int fv = shape.mesh.num_face_vertices[f];

            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

                VertexKey key{idx.vertex_index, idx.texcoord_index, idx.normal_index};

                if (uniqueVertices.count(key) == 0) {
                    uniqueVertices[key] = static_cast<uint32_t>(vertices.size() / (hasTexCoords ? 5 : 3));

                    // position
                    vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
                    vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
                    vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);

                    // texture stuff if available
                    if (hasTexCoords && idx.texcoord_index >= 0) {
                        vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
                        vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
                    }
                }

                indices.push_back(uniqueVertices[key]);
            }
            index_offset += fv;
        }
    }

    auto mesh = std::make_shared<Mesh>(vertices, indices);
    // mesh->m_hasTexCoords = hasTexCoords;

    return mesh;
}

auto MeshLoader::getModelPath(const std::string& filename) -> std::filesystem::path {
    return std::filesystem::path("resources/models") / filename;
}

}  // namespace Vengine