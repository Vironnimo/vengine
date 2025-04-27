#pragma once

namespace Vengine {

struct VertexLayout {
    bool hasPosition = true;
    bool hasTexCoords = false;
    bool hasNormals = false;

    [[nodiscard]] auto calculateStride() const -> int {
        int stride = 0;
        if (hasPosition) {
            stride += 3 * sizeof(float);
        }
        if (hasTexCoords) {
            stride += 2 * sizeof(float);
        }
        if (hasNormals) {
            stride += 3 * sizeof(float);
        }
        return stride;
    }
};

}  // namespace Vengine