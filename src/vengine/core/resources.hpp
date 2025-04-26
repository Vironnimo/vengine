#pragma once

#include <string>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <miniaudio.h>
#include <stb_image.h>

namespace Vengine {

class IResource {
   public:
    virtual ~IResource() = default;
    virtual auto load(const std::string& fileName) -> bool = 0;
    virtual auto unload() -> bool = 0;

    [[nodiscard]] auto isLoaded() const -> bool {
        return m_isLoaded;
    }

   protected:
    bool m_isLoaded = false;
};

class Texture : public IResource {
   public:
    auto load(const std::string& fileName) -> bool override {
        auto folder = std::filesystem::path("resources/textures");
        auto fullPath = folder / fileName;

        // stbi_set_flip_vertically_on_load(true);  // do we need this?
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // without this we have rainbowcolors i feel like

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

class Sound : public IResource {
   public:
    ~Sound() override {
        unload();
    }

    void setEngine(ma_engine* engine) {
        m_engine = engine;
    }

    auto load(const std::string& fileName) -> bool override {
        if (!m_engine) {
            spdlog::error("Audio engine not initialized for sound: {}", fileName);
            return false;
        }
        auto folder = std::filesystem::path("resources/sounds");
        auto fullPath = folder / fileName;

        ma_result result = ma_sound_init_from_file(m_engine, fullPath.string().c_str(), 0, nullptr, nullptr, &m_sound);

        if (result != MA_SUCCESS) {
            spdlog::error("Failed to load sound: {} (Error code: {})", fullPath.string());
            return false;
        }

        m_isLoaded = true;
        return true;
    }

    auto unload() -> bool override {
        if (!m_isLoaded) {
            return false;
        }

        ma_sound_uninit(&m_sound);
        m_isLoaded = false;
        return true;
    }

    auto play() -> bool {
        if (!m_isLoaded || !m_engine) {
            return false;
        }

        ma_result result = ma_sound_start(&m_sound);
        if (result != MA_SUCCESS) {
            spdlog::error("Failed to play sound (Error code: {})");
            return false;
        }

        return true;
    }

    auto stop() -> bool {
        if (!m_isLoaded) {
            return false;
        }

        ma_result result = ma_sound_stop(&m_sound);
        if (result != MA_SUCCESS) {
            spdlog::error("Failed to stop sound (Error code: {})");
            return false;
        }

        return true;
    }

    auto setVolume(float volume) -> void {
        if (m_isLoaded) {
            ma_sound_set_volume(&m_sound, volume);
        }
    }

    auto setPan(float pan) -> void {
        if (m_isLoaded) {
            ma_sound_set_pan(&m_sound, pan);
        }
    }

    auto isPlaying() -> bool {
        if (!m_isLoaded) {
            return false;
        }

        ma_bool32 isPlaying = ma_sound_is_playing(&m_sound) ? MA_TRUE : MA_FALSE;
        return isPlaying == MA_TRUE;
    }

   private:
    ma_sound m_sound{};
    ma_engine* m_engine{};
};

}  // namespace Vengine