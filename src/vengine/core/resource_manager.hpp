#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <tl/expected.hpp>
#include "vengine/core/error.hpp"
#include "vengine/core/thread_manager.hpp"

#include "vengine/core/mesh_loader.hpp"
#include "resources.hpp"
#include <typeindex>
#include <unordered_map>
#include <glad/glad.h>
#include <spdlog/spdlog.h>

namespace Vengine {

struct LoadTask {
    std::string name;
    std::string fileName;
    std::string type;
};

// TODO split resources map into different types
class ResourceManager {
   public:
    ResourceManager(std::shared_ptr<ThreadManager> threadManager);
    ~ResourceManager();
    auto init() -> tl::expected<void, Error>;

    template <typename T>
    auto load(const std::string& name, const std::string& fileName) -> bool {
        if constexpr (std::is_same_v<T, Mesh>) {
            // auto mesh = m_meshLoader->loadFromObj(fileName);
            // if (!mesh) {
            //     spdlog::error("Failed to load mesh: {}", fileName);
            //     return false;
            // }

            // if (mesh->load(fileName)) {
            //     std::lock_guard<std::mutex> lock(m_resourceMutex);
            //     m_resources[std::type_index(typeid(T))][name] = mesh;

            //     return true;
            // }
        }

        auto resource = std::make_shared<T>();

        if constexpr (std::is_same_v<T, Sound>) {
            resource->setEngine(&m_audioEngine);
        }

        if (resource->load(fileName)) {
            std::lock_guard<std::mutex> lock(m_resourceMutex);
            m_resources[std::type_index(typeid(T))][name] = resource;
            return true;
        }

        return false;
    }

    template <typename T>
    auto loadAsync(const std::string& name, const std::string& fileName) -> void {
        m_threadManager->enqueueTask(
            [this, name, fileName]() {
                spdlog::debug("Loading resource: {} from file: {}", name, fileName);
                auto resource = std::make_shared<T>();

                if constexpr (std::is_same_v<T, Mesh>) {
                    if (fileName == "buildin.plane") {
                        // TODO where do i we get the dimensions from?
                        // also really weird with the strings i think, just an enum for build ins?
                        resource = m_meshLoader->createPlane();
                    } else {
                        resource = m_meshLoader->loadFromObj(fileName);
                    }
                    if (!resource) {
                        spdlog::error("Failed to load mesh: {}", fileName);
                        return;
                    }
                }

                if constexpr (std::is_same_v<T, Sound>) {
                    resource->setEngine(&m_audioEngine);
                }

                if (resource->load(fileName)) {
                    {
                        std::lock_guard<std::mutex> lock(m_resourceMutex);
                        m_resources[std::type_index(typeid(T))][name] = resource;
                    }

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
                    } else if constexpr (std::is_same_v<T, Mesh>) {
                        auto mesh = std::static_pointer_cast<Mesh>(resource);
                        if (mesh->needsGpuInit()) {
                            m_threadManager->enqueueMainThreadTask(
                                [mesh, name]() {
                                    if (mesh->finalizeOnGpu()) {
                                        // spdlog::info("Finalized mesh on GPU: {}", name);
                                    } else {
                                        spdlog::error("Failed to finalize mesh on GPU: {}", name);
                                    }
                                },
                                "Finalize mesh: " + name);
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

        // replace with any_of? it's c++20
        for (const auto& [type, resources] : m_resources) {
            if (resources.find(name) != resources.end()) {
                return true;
            }
        }

        return false;
    }

    template <typename T>
    auto get(const std::string& name) -> std::shared_ptr<T> {
        std::lock_guard<std::mutex> lock(m_resourceMutex);
        auto typeIt = m_resources.find(std::type_index(typeid(T)));
        if (typeIt != m_resources.end()) {
            auto& innerMap = typeIt->second;
            auto it = innerMap.find(name);
            if (it != innerMap.end()) {
                return std::static_pointer_cast<T>(it->second);
            }
        }
        return nullptr;
    }

   private:
    std::filesystem::path m_resourceRoot;
    std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<IResource>>> m_resources;

    std::shared_ptr<ThreadManager> m_threadManager;
    std::mutex m_resourceMutex;

    std::unique_ptr<MeshLoader> m_meshLoader;

    ma_engine m_audioEngine;
};

}  // namespace Vengine