#include "mesh_loader.hpp"
#include <cstddef>

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
    const auto& materials = reader.GetMaterials();

    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    bool hasTexCoords = !attrib.texcoords.empty();
    bool hasNormals = !attrib.normals.empty();
    int floatsPerVertex = hasTexCoords ? 5 : 3;
    if (hasNormals) {
        floatsPerVertex += 3;
    }
    // spdlog::debug("Loading OBJ: '{}'. Has TexCoords: {}, Has Normals: {}, FloatsPerVertex: {}", filename, hasTexCoords,
    // hasNormals,
    //               floatsPerVertex);

    // helper thingy to make sure we don't have duplicate vertices
    struct VertexKey {
        int v_idx, vt_idx, vn_idx;

        auto operator<(const VertexKey& other) const -> bool {
            return std::tie(v_idx, vt_idx, vn_idx) < std::tie(other.v_idx, other.vt_idx, other.vn_idx);
        }
        // add equality operator for map lookup
        bool operator==(const VertexKey& other) const {
            return std::tie(v_idx, vt_idx, vn_idx) == std::tie(other.v_idx, other.vt_idx, other.vn_idx);
        }
    };

    std::map<VertexKey, uint32_t> uniqueVertices;
    struct SubmeshInfo {
        uint32_t indexOffset;
        uint32_t indexCount;
        std::string materialName;
    };
    std::vector<SubmeshInfo> submeshes;

    for (const auto& shape : shapes) {
        size_t index_offset = 0;
        int last_material_id = -1;
        size_t submesh_start = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            size_t fv = shape.mesh.num_face_vertices[f];
            int material_id = shape.mesh.material_ids.empty() ? -1 : shape.mesh.material_ids[f];

            // start new submesh
            if (material_id != last_material_id && f > 0) {
                SubmeshInfo submesh;
                submesh.indexOffset = static_cast<uint32_t>(submesh_start * 3);  // 3 indices per face
                submesh.indexCount = static_cast<uint32_t>((f - submesh_start) * 3);
                submesh.materialName =
                    (last_material_id >= 0 && last_material_id < materials.size()) ? materials[last_material_id].name : "";
                submeshes.push_back(submesh);
                submesh_start = f;
            }
            last_material_id = material_id;

            if (fv != 3) {
                spdlog::warn("Face {} in shape '{}' is not a triangle ({} vertices), skipping.", f, shape.name, fv);
                index_offset += fv;
                continue;
            }

            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

                VertexKey key;
                key.v_idx = idx.vertex_index;
                key.vt_idx = idx.texcoord_index;
                key.vn_idx = idx.normal_index;

                if (uniqueVertices.find(key) == uniqueVertices.end()) {
                    // new, add it
                    uniqueVertices[key] = static_cast<uint32_t>(vertices.size() / static_cast<size_t>(floatsPerVertex));

                    // 1. Position (always present)
                    vertices.push_back(attrib.vertices[3 * static_cast<size_t>(key.v_idx) + 0]);
                    vertices.push_back(attrib.vertices[3 * static_cast<size_t>(key.v_idx) + 1]);
                    vertices.push_back(attrib.vertices[3 * static_cast<size_t>(key.v_idx) + 2]);

                    // 2. Texture Coords (if available in OBJ and vertex)
                    if (hasTexCoords) {
                        if (key.vt_idx >= 0) {
                            vertices.push_back(attrib.texcoords[2 * static_cast<size_t>(key.vt_idx) + 0]);
                            vertices.push_back(attrib.texcoords[2 * static_cast<size_t>(key.vt_idx) + 1]);
                        } else {
                            vertices.push_back(0.0f);  // add dummy tex coords if there are none, is this a mistake?
                            vertices.push_back(0.0f);
                        }
                    }

                    // 3. Normals (if available in OBJ and vertex)
                    if (hasNormals) {
                        if (key.vn_idx >= 0) {
                            vertices.push_back(attrib.normals[3 * static_cast<size_t>(key.vn_idx) + 0]);
                            vertices.push_back(attrib.normals[3 * static_cast<size_t>(key.vn_idx) + 1]);
                            vertices.push_back(attrib.normals[3 * static_cast<size_t>(key.vn_idx) + 2]);
                        } else {
                            // again dummy data, probably a mistake
                            vertices.push_back(0.0f);
                            vertices.push_back(1.0f);
                            vertices.push_back(0.0f);
                            spdlog::warn("Vertex missing normal index in OBJ, using default normal.");
                        }
                    }
                }
                indices.push_back(uniqueVertices[key]);
            }
            index_offset += fv;
        }
        // Add last submesh for this shape
        if (shape.mesh.num_face_vertices.size() > 0) {
            SubmeshInfo submesh;
            submesh.indexOffset = static_cast<uint32_t>(submesh_start * 3);
            submesh.indexCount = static_cast<uint32_t>((shape.mesh.num_face_vertices.size() - submesh_start) * 3);
            submesh.materialName =
                (last_material_id >= 0 && last_material_id < materials.size()) ? materials[last_material_id].name : "";
            submeshes.push_back(submesh);
        }
    }

    VertexLayout layout;
    layout.hasPosition = true;
    layout.hasTexCoords = hasTexCoords;
    layout.hasNormals = hasNormals;

    auto mesh = std::make_shared<Mesh>(vertices, indices, layout);

    // add submeshes to mesh
    for (const auto& s : submeshes) {
        Submesh submesh;
        submesh.indexOffset = s.indexOffset;
        submesh.indexCount = s.indexCount;
        submesh.materialName = s.materialName;
        mesh->addSubmesh(submesh);
        spdlog::debug("Submesh: {} indices, material: {}", submesh.indexCount, submesh.materialName);
    }

    return mesh;
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