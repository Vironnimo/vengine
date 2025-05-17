#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <tl/expected.hpp>
#include "vengine/core/error.hpp"
#include "vengine/core/thread_manager.hpp"
#include "vengine/core/model_loader.hpp"

#include "vengine/core/mesh_loader.hpp"
#include "resources.hpp"
#include <typeindex>
#include <tuple>
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
    ResourceManager();
    ~ResourceManager();
    auto init(std::shared_ptr<ThreadManager> threadManager) -> tl::expected<void, Error>;

    template <typename T, typename... Args>
    auto load(const std::string& name, const std::string& fileName, Args&&... loadArgs) -> bool {
        assert(!fileName.empty() && "Filename cannot be empty");
        assert(!name.empty() && "Name cannot be empty");

        // auto argsSize = sizeof...(loadArgs);
        spdlog::debug("Loading resource: {} from file: {}", name, fileName);
        std::shared_ptr<T> resource;

        if constexpr (std::is_same_v<T, Mesh>) {
            if (fileName == "buildin.plane") {
                // TODO what if we have more than 4 args?
                // resource = std::apply(
                //     [this](auto&&... args) { return m_meshLoader->createPlane(std::forward<decltype(args)>(args)...); },
                //     loadArgsTuple);
            } else {
                resource = m_meshLoader->loadModel(fileName);
            }
            if (!resource) {
                spdlog::error("Failed to load mesh: {}", fileName);
                return false;
            }
        }

        if constexpr (std::is_same_v<T, Shader>) {
            resource = std::make_shared<T>(name, fileName, std::get<0>(std::tuple<Args...>(loadArgs...)));
        }

        if (!resource) {
            resource = std::make_shared<T>();
        }

        if constexpr (std::is_same_v<T, Sound>) {
            resource->setEngine(&m_audioEngine);
        }

        if (resource->load(fileName)) {
            {
                std::lock_guard<std::mutex> lock(m_resourceMutex);
                m_resources[std::type_index(typeid(T))][name] = resource;
            }

            if (resource->needsMainThreadInit()) {
                resource->finalizeOnMainThread();
            }
        } else {
            spdlog::error("Failed to load {} resource: {}", typeid(T).name(), name);
            return false;
        }
        return true;
    }

    template <typename T, typename... Args>
    auto loadAsync(const std::string& name, const std::string& fileName, Args&&... loadArgs) -> void {
        assert(!fileName.empty() && "Filename cannot be empty");
        assert(!name.empty() && "Name cannot be empty");

        auto argsSize = sizeof...(loadArgs);
        m_threadManager->enqueueTask(
            [this, name, fileName, argsSize, loadArgsTuple = std::make_tuple(std::forward<Args>(loadArgs)...)]() {
                spdlog::debug("Loading resource: {} from file: {}", name, fileName);
                std::shared_ptr<T> resource;

                if constexpr (std::is_same_v<T, Mesh>) {
                    if (fileName == "buildin.plane") {
                        // TODO what if we have more than 4 args?
                        resource = std::apply(
                            [this](auto&&... args) {
                                return m_meshLoader->createPlane(std::forward<decltype(args)>(args)...);
                            },
                            loadArgsTuple);
                    } else {
                        resource = m_meshLoader->loadModel(fileName);
                    }
                    if (!resource) {
                        spdlog::error("Failed to load mesh: {}", fileName);
                        return;
                    }
                }
                if constexpr (std::is_same_v<T, Shader>) {
                    resource = std::make_shared<T>(name, fileName, std::get<0>(loadArgsTuple));
                }

                if (!resource) {
                    resource = std::make_shared<T>();
                }

                if constexpr (std::is_same_v<T, Sound>) {
                    resource->setEngine(&m_audioEngine);
                }

                if (resource->load(fileName)) {
                    {
                        std::lock_guard<std::mutex> lock(m_resourceMutex);
                        m_resources[std::type_index(typeid(T))][name] = resource;
                    }

                    if (resource->needsMainThreadInit()) {
                        m_threadManager->enqueueMainThreadTask(
                            [resource, name]() {
                                if (resource->finalizeOnMainThread()) {
                                    // spdlog::info("Finalized resource on main thread: {}", name);
                                } else {
                                    spdlog::error("Failed to finalize resource on main thread: {}", name);
                                }
                            },
                            "Finalize resource: " + name);
                    }
                } else {
                    spdlog::error("Failed to load {} resource: {}", typeid(T).name(), name);
                }
            },
            "Load " + std::string(typeid(T).name()) + ": " + name);
    }

    template <typename T>
    auto add(const std::string& name, std::shared_ptr<T> resource) -> void {
        std::lock_guard<std::mutex> lock(m_resourceMutex);
        m_resources[std::type_index(typeid(T))][name] = resource;
    }

    auto isLoaded(const std::string& name) -> bool {
        std::lock_guard<std::mutex> lock(m_resourceMutex);

        return std::any_of(m_resources.begin(), m_resources.end(), [&name](const auto& pair) {
            const auto& resources = pair.second;
            return resources.find(name) != resources.end();
        });
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

    auto loadModel(const std::string& name,
                   const std::string& fileName,
                   std::shared_ptr<Shader> defaultShader = nullptr) -> bool;
    auto loadModelAsync(const std::string& name,
                        const std::string& fileName,
                        std::shared_ptr<Shader> defaultShader = nullptr) -> void;

   private:
    std::filesystem::path m_resourceRoot;
    std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<IResource>>> m_resources;

    std::shared_ptr<ThreadManager> m_threadManager;
    std::mutex m_resourceMutex;

    std::shared_ptr<MeshLoader> m_meshLoader;
    std::unique_ptr<ModelLoader> m_modelLoader;

    ma_engine m_audioEngine;
};

}  // namespace Vengine