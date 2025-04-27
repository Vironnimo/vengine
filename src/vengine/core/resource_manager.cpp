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

ResourceManager::ResourceManager(std::shared_ptr<ThreadManager> threadManager) : m_threadManager(std::move(threadManager)) {
    spdlog::debug("Constructor ResourceManager");
}

auto ResourceManager::init() -> tl::expected<void, Error> {
    m_resourceRoot = std::filesystem::path("resources");

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

    for (const auto& [id, resource] : m_resources) {
        auto* res = static_cast<IResource*>(resource.get());
        if (res != nullptr) {
            res->unload();
        }
    }

    ma_engine_uninit(&m_audioEngine);
}

}  // namespace Vengine