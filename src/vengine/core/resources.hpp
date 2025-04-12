#pragma once

#include <string>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Vengine {

class IResource {
   public:
    virtual ~IResource() = default;
    virtual auto load(const std::string& fileName) -> bool = 0;
    virtual auto unload() -> bool = 0;

   protected:
    bool m_isLoaded = false;
};

class Texture : public IResource {
   public:
    auto load(const std::string& fileName) -> bool override {
        auto folder = std::filesystem::path("resources/textures");
        auto fullPath = folder / fileName;

        // stbi_set_flip_vertically_on_load(true);  // do we need this?
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // without this we have rainbowcolors i feel like

        unsigned char* textureData = stbi_load(fullPath.string().c_str(), &m_width, &m_height, &m_channels, 0);
        if (textureData == nullptr) {
            spdlog::error("Failed to load texture: {}", fullPath.string());
            return false;
        }

        glGenTextures(1, &m_textureID);
        glBindTexture(GL_TEXTURE_2D, m_textureID);

        if (m_channels == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
        } else if (m_channels == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
        } else {
            spdlog::error("Unknown number of channels: {}", m_channels);
            stbi_image_free(textureData);
            return false;
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(textureData);

        glBindTexture(GL_TEXTURE_2D, 0);

        m_isLoaded = true;
        return true;
    }

    auto unload() -> bool override {
        if (!m_isLoaded) {
            return false;
        }
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
        m_isLoaded = false;

        return true;
    }

    [[nodiscard]] auto getTextureID() const -> GLuint {
        return m_textureID;
    }

    [[nodiscard]] auto getWidth() const -> int {
        return m_width;
    }

    [[nodiscard]] auto getHeight() const -> int {
        return m_height;
    }

   private:
    GLuint m_textureID = 0;
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;
};

}