#include "resource_manager.hpp"

// note might be wrong here, we'll see
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif

#include <spdlog/spdlog.h>
#include <tl/expected.hpp>
#include <vengine/core/error.hpp>
#include "resources.hpp"

namespace Vengine {

ResourceManager::ResourceManager() {
    spdlog::debug("Constructor ResourceManager");
    init();
}

auto ResourceManager::init() -> tl::expected<void, Error> {
    m_resourceRoot = std::filesystem::path("resources");

    if (!std::filesystem::exists(m_resourceRoot)) {
        spdlog::warn("Resource root does not exist: {}", m_resourceRoot.string());
        return tl::unexpected(Error{"Resource root does not exist"});
    }
    // sdl init
    // if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    //     spdlog::warn("failed to init sdl: {}", SDL_GetError());
    // }
    // // sdl_mixer init
    // if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    //     spdlog::warn("failed to init sdl mixer: {}", Mix_GetError());
    //     SDL_Quit();
    // }
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

    // Mix_CloseAudio();
    // SDL_Quit();
}

}  // namespace Vengine