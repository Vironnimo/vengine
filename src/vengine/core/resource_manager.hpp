#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <tl/expected.hpp>
#include "vengine/core/error.hpp"
#include <unordered_map>

#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <spdlog/spdlog.h>

namespace Vengine {

class ResourceManager {
    // todo create method for adding a resource, but only load it once it's actually used (lazy load)
   public:
    ResourceManager();
    ~ResourceManager();
    auto init() -> tl::expected<void, Error>;

    template <typename T>
    auto load(const std::string& name, const std::string& fileName) -> bool {
        auto resource = std::make_shared<T>();
        if (resource->load(fileName)) {
            m_resources[name] = resource;
            return true;
        }

        return false;
    }

    template <typename T>
    auto get(const std::string& name) -> T* {
        auto it = m_resources.find(name);
        if (it != m_resources.end()) {
            return static_cast<T*>(it->second.get());
        }

        return nullptr;
    }

   private:
    std::filesystem::path m_resourceRoot;

    std::unordered_map<std::string, std::shared_ptr<void>> m_resources;
};

}  // namespace Vengine