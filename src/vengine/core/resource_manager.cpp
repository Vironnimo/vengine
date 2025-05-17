#include "resource_manager.hpp"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include <stb_image.h>

#include <spdlog/spdlog.h>
#include <tl/expected.hpp>
#include <vengine/core/error.hpp>

#include "resources.hpp"

namespace Vengine {

ResourceManager::ResourceManager() {
    spdlog::debug("Constructor ResourceManager");
}

auto ResourceManager::init(std::shared_ptr<ThreadManager> threadManager) -> tl::expected<void, Error> {
    assert(threadManager != nullptr && "ThreadManager cannot be null");
    m_resourceRoot = std::filesystem::path("resources");

    m_meshLoader = std::make_shared<MeshLoader>();
    m_modelLoader = std::make_unique<ModelLoader>(m_meshLoader, this);
    // NOWUSETHEOMDELLOADER GOGOGOGOGOGO
    m_threadManager = std::move(threadManager);

    if (!std::filesystem::exists(m_resourceRoot)) {
        return tl::unexpected(Error{"Resource root does not exist"});
    }

    ma_result result = ma_engine_init(nullptr, &m_audioEngine);
    if (result != MA_SUCCESS) {
        return tl::unexpected(Error{"Failed to initialize audio engine"});
    }

    return {};
}

ResourceManager::~ResourceManager() {
    spdlog::debug("Destructor ResourceManager");

    for (auto& [type, resources] : m_resources) {
        for (auto& [name, resource] : resources) {
            resource->unload();
        }
    }

    ma_engine_uninit(&m_audioEngine);
}

auto ResourceManager::loadModel(const std::string& name, const std::string& fileName, 
                               std::shared_ptr<Shader> defaultShader) -> bool {
    assert(!fileName.empty() && "Filename cannot be empty");
    assert(!name.empty() && "Name cannot be empty");
    
    spdlog::debug("Loading model: {} from file: {}", name, fileName);
    
    auto model = m_modelLoader->loadModel(fileName, defaultShader);
    if (!model) {
        spdlog::error("Failed to load model: {}", fileName);
        return false;
    }
    
    {
        std::lock_guard<std::mutex> lock(m_resourceMutex);
        m_resources[std::type_index(typeid(Model))][name] = model;
    }
    
    if (model->needsMainThreadInit()) {
        model->finalizeOnMainThread();
    }
    
    return true;
}

auto ResourceManager::loadModelAsync(const std::string& name, const std::string& fileName, 
                                    std::shared_ptr<Shader> defaultShader) -> void {
    assert(!fileName.empty() && "Filename cannot be empty");
    assert(!name.empty() && "Name cannot be empty");
    
    spdlog::debug("Loading model async: {} from file: {}", name, fileName);
    
    m_threadManager->enqueueTask([this, name, fileName, defaultShader]() {
        auto model = m_modelLoader->loadModel(fileName, defaultShader);
        if (!model) {
            spdlog::error("Failed to load model: {}", fileName);
            return;
        }
        
        {
            std::lock_guard<std::mutex> lock(m_resourceMutex);
            m_resources[std::type_index(typeid(Model))][name] = model;
        }
        
        // if (model->needsMainThreadInit()) {
            m_threadManager->enqueueMainThreadTask([model, name]() {
                model->finalizeOnMainThread();
            }, "Finalize model: " + name);
        // }
    });
}

}  // namespace Vengine