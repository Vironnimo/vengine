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

    m_meshLoader = std::make_unique<MeshLoader>();
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

}  // namespace Vengine