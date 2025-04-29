#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <tl/expected.hpp>
#include "vengine/core/error.hpp"
#include "vengine/core/thread_manager.hpp"

#include "resources.hpp"
#include <unordered_map>
#include <glad/glad.h>
#include <spdlog/spdlog.h>

namespace Vengine {

struct LoadTask {
    std::string name;
    std::string fileName;
    std::string type;
};

// todo split resources map into different types
class ResourceManager {
   public:
    ResourceManager(std::shared_ptr<ThreadManager> threadManager);
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
    auto loadAsync(const std::string& name, const std::string& fileName) -> void {
        m_threadManager->enqueueTask(
            [this, name, fileName]() {
                auto resource = std::make_shared<T>();

                if constexpr (std::is_same_v<T, Sound>) {
                    resource->setEngine(&m_audioEngine);
                }

                if (resource->load(fileName)) {
                    // spdlog::info("Successfully loaded {} resource: {}", typeid(T).name(), name);
                    {
                        std::lock_guard<std::mutex> lock(m_resourceMutex);
                        m_resources[name] = resource;
                    }

                    // add task to main thread queue, cause for opengl it needs to be done on the main thread
                    if constexpr (std::is_same_v<T, Texture>) {
                        auto texture = std::static_pointer_cast<Texture>(resource);
                        if (texture->needsGpuInit()) {
                            m_threadManager->enqueueMainThreadTask(
                                [texture, name]() {
                                    if (texture->finalizeOnGpu()) {
                                        // spdlog::info("Finalized texture on GPU: {}", name);
                                    } else {
                                        spdlog::error("Failed to finalize texture on GPU: {}", name);
                                    }
                                },
                                "Finalize texture: " + name);
                        }
                    }
                } else {
                    spdlog::error("Failed to load {} resource: {}", typeid(T).name(), name);
                }
            },
            "Load " + std::string(typeid(T).name()) + ": " + name);
    }

    auto isLoaded(const std::string& name) -> bool {
        std::lock_guard<std::mutex> lock(m_resourceMutex);
        return m_resources.find(name) != m_resources.end();
    }

    template <typename T>
    auto get(const std::string& name) -> std::shared_ptr<T> {
        std::lock_guard<std::mutex> lock(m_resourceMutex);
        auto it = m_resources.find(name);
        if (it != m_resources.end()) {
            return std::static_pointer_cast<T>(it->second);
        }

        return nullptr;
    }

   private:
    std::filesystem::path m_resourceRoot;
    std::unordered_map<std::string, std::shared_ptr<void>> m_resources;
    ma_engine m_audioEngine;

    std::shared_ptr<ThreadManager> m_threadManager;
    std::mutex m_resourceMutex;
};

}  // namespace Vengine