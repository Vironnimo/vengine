#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <tl/expected.hpp>
#include "vengine/core/error.hpp"

// #define MINIAUDIO_IMPLEMENTATION
// #include <miniaudio.h>
// #ifndef STB_IMAGE_IMPLEMENTATION
// #define STB_IMAGE_IMPLEMENTATION
// #endif
// #include <stb_image.h>

#include "resources.hpp"
#include <unordered_map>
#include <glad/glad.h>
#include <spdlog/spdlog.h>

namespace Vengine {

class ResourceManager {
    // todo extra method for loading resources lazy?
   public:
    ResourceManager();
    ~ResourceManager();
    auto init() -> tl::expected<void, Error>;

    template <typename T>
    auto load(const std::string& name, const std::string& fileName) -> bool {
        auto resource = std::make_shared<T>();
        if constexpr (std::is_same_v<T, Sound>) {
            resource->setEngine(&m_audioEngine);
        }
        if (resource->load(fileName)) {
            m_resources[name] = resource;
            return true;
        }

        return false;
    }

    template <typename T>
    auto get(const std::string& name) -> std::shared_ptr<T> {
        auto it = m_resources.find(name);
        if (it != m_resources.end()) {
            // return static_cast<T*>(it->second.get());
            return std::static_pointer_cast<T>(it->second);
        }

        return nullptr;
    }

   private:
    std::filesystem::path m_resourceRoot;
    std::unordered_map<std::string, std::shared_ptr<void>> m_resources;
    ma_engine m_audioEngine;  
};

}  // namespace Vengine