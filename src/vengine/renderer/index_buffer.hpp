#pragma once
#include <glad/glad.h>
#include <cstdint>

namespace Vengine {

class IndexBuffer {
public:
    IndexBuffer(const uint32_t* indices, uint32_t count);
    ~IndexBuffer();

    auto bind() const -> void;
    auto unbind() const -> void;
    
    [[nodiscard]] auto getCount() const -> uint32_t; 

private:
    GLuint m_id = 0;
    uint32_t m_count;
};

}  // namespace Vengine