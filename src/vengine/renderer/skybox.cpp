#include "skybox.hpp"
#include <spdlog/spdlog.h>
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include "vengine/renderer/vertex_layout.hpp"

namespace Vengine {

Skybox::Skybox() {
    // (36 vertices total - 6 faces, 2 triangles per face, 3 vertices per triangle)
    float skyboxVertices[] = {-1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
                              1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

                              -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
                              -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

                              1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

                              -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

                              -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

                              -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
                              1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

    m_vao = std::make_shared<VertexArray>();
    m_vbo = std::make_shared<VertexBuffer>(skyboxVertices, sizeof(skyboxVertices));

    m_vao->bind();
    m_vbo->bind();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    m_vao->unbind();
    m_vbo->unbind();
}

Skybox::~Skybox() {
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
    }
}

auto Skybox::loadFromTextures(const std::vector<std::shared_ptr<Texture>>& textures) -> bool {
    if (textures.size() != 6) {
        spdlog::error("Skybox requires exactly 6 textures");
        return false;
    }

    // cubemap texture for skybox
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);

    for (unsigned int i = 0; i < textures.size(); ++i) {
        const auto& texture = textures[i];
        if (!texture || !texture->isLoaded()) {
             spdlog::error("Skybox face texture at index {} is not loaded or invalid.", i);
             glDeleteTextures(1, &m_textureID); 
             m_textureID = 0;
             return false;
        }

        auto rawData = texture->getRawData();

        if (rawData && rawData->pixels) {
            GLenum format = (rawData->channels == 4) ? GL_RGBA : GL_RGB;
            GLint internalFormat = (rawData->channels == 4) ? GL_RGBA8 : GL_RGB8;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, rawData->width, rawData->height, 0, format, GL_UNSIGNED_BYTE, rawData->pixels);
            // spdlog::debug("Loaded skybox face {} from texture ({}x{}, {} channels)", i, rawData->width, rawData->height, rawData->channels);
        } else {
            spdlog::error("Failed to get raw image data for skybox texture at index {}", i);
            //  glDeleteTextures(1, &m_textureID); 
            //  m_textureID = 0;
            // return false;
        }
    }

    // set texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0); 
    return true;
}

auto Skybox::load(const std::vector<std::string>& faceFiles) -> bool {
    if (faceFiles.size() != 6) {
        spdlog::error("Skybox requires exactly 6 face textures");
        return false;
    }

    // cubemap texture for skybox
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);

    // todo should use resource manager here i guess.
    int width;
    int height;
    int nrChannels;
    for (unsigned int i = 0; i < faceFiles.size(); i++) {
        unsigned char* data = stbi_load(faceFiles[i].c_str(), &width, &height, &nrChannels, 0);
        if (data != nullptr) {
            GLenum format = nrChannels == 4 ? GL_RGBA : GL_RGB;
            GLint internalFormat = nrChannels == 4 ? GL_RGBA8 : GL_RGB8;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE,
                         data);
            stbi_image_free(data);
        } else {
            spdlog::error("Failed to load cubemap texture: {}", faceFiles[i]);
            stbi_image_free(data);
            return false;
        }
    }

    // set texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return true;
}

auto Skybox::render(const glm::mat4& view, const glm::mat4& projection) -> void {
    // save state
    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    GLint depthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);

    // change depth so skybox passes depth test at maximum depth
    glDepthFunc(GL_LEQUAL);

    m_shader->bind();

    // remove translation from view matrix (only keep rotation)
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));

    m_shader->setUniformMat4("projection", projection);
    m_shader->setUniformMat4("view", skyboxView);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);

    m_vao->bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
    m_vao->unbind();

    // restore state
    if (static_cast<bool>(depthTestEnabled)) {
        glDepthFunc(depthFunc);
    }
}

auto Skybox::setShader(std::shared_ptr<Shader> shader) -> void {
    m_shader = std::move(shader);
}

auto Skybox::unload() -> void {
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }
}

}  // namespace Vengine