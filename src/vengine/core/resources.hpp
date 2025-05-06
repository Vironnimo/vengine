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
    struct RawImageData {
        unsigned char* pixels = nullptr;
        int width = 0;
        int height = 0;
        int channels = 0;
    };

    auto load(const std::string& fileName) -> bool override {
        auto folder = std::filesystem::path("resources/textures");
        auto fullPath = folder / fileName;

        if (!m_rawData) {
            m_rawData = std::make_shared<RawImageData>();
        }

        m_rawData->pixels = stbi_load(fullPath.string().c_str(), &m_rawData->width, &m_rawData->height, &m_rawData->channels, 0);

        if (!m_rawData->pixels) {
            spdlog::error("Failed to load texture from file: {}", fullPath.string());
            return false;
        }

        m_width = m_rawData->width;
        m_height = m_rawData->height;
        m_needsGpuInit = true;
        m_isLoaded = true;
        // spdlog::info("Loaded texture data from: {}", fullPath.string());

        return true;
    }

    // send data to gpu
    auto finalizeOnGpu() -> bool {
        if (!m_needsGpuInit || !m_rawData || !m_rawData->pixels) {
            return false;
        }

        if (m_id == 0) {
            glGenTextures(1, &m_id);
        }

        glBindTexture(GL_TEXTURE_2D, m_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLenum format = (m_rawData->channels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, m_rawData->width, m_rawData->height, 0, format, GL_UNSIGNED_BYTE, m_rawData->pixels);
        glGenerateMipmap(GL_TEXTURE_2D);

        // TODO reactivate i guess. for now the skybox needs the data on reload the scene
        // free memory after upload
        // stbi_image_free(m_rawData->pixels);
        // m_rawData.reset();

        m_needsGpuInit = false;
        // spdlog::info("Texture finalized on GPU, ID: {}", m_id);
        return true;
    }

    auto unload() -> bool override {
        if (!m_isLoaded) {
            return false;
        }
        glDeleteTextures(1, &m_id);
        m_id = 0;
        m_isLoaded = false;

        return true;
    }

    [[nodiscard]] auto getTextureID() const -> GLuint {
        return m_id;
    }

    [[nodiscard]] auto getRawData() const -> std::shared_ptr<RawImageData> {
        return m_rawData;
    }

    [[nodiscard]] auto getPixels() const -> unsigned char* {
        return m_rawData ? m_rawData->pixels : nullptr;
    }

    [[nodiscard]] auto getFormat() const -> GLenum {
        return (m_channels == 4) ? GL_RGBA : GL_RGB;
    }

    [[nodiscard]] auto getChannels() const -> int {
        return m_channels;
    }

    [[nodiscard]] auto getWidth() const -> int {
        return m_width;
    }

    [[nodiscard]] auto getHeight() const -> int {
        return m_height;
    }

    [[nodiscard]] auto needsGpuInit() const -> bool {
        return m_needsGpuInit;
    }

   private:
    GLuint m_id = 0;
    int m_width = 0;
    int m_height = 0;
    std::shared_ptr<RawImageData> m_rawData;
    bool m_needsGpuInit = false;
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
            // why do we need do it like this here with cpp23?
            spdlog::error(fmt::runtime("Failed to load sound: {} (Error code: {})"), fullPath.string());
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