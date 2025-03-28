#include "vengine.hpp"

#include <spdlog/spdlog.h>

#include <memory>
#include <tl/expected.hpp>

#include "vengine/renderer/renderer.hpp"
#include "vengine/core/resource_manager.hpp"

namespace Vengine {

Vengine::Vengine() {
    auto result = init();
    if (!result) {
        spdlog::info("{}", result.error().message);
    }
}

[[nodiscard]] auto Vengine::init() -> tl::expected<void, Error> {
    // tl::expected<void, Error> Vengine::init() {
    window = std::make_shared<Window>();
    if (auto result = window->init(); !result) {
        return tl::unexpected(result.error());
    }
    WindowParams params;
    if (auto result = window->create(params); !result) {
        return tl::unexpected(result.error());
    }

    renderer = std::make_unique<Renderer>();
    if (auto result = renderer->init(window); !result) {
        return tl::unexpected(result.error());
    }

    resourceManager = std::make_unique<ResourceManager>();
    if (auto result = resourceManager->init(); !result) {
        return tl::unexpected(result.error());
    }

    return {};
}

auto Vengine::run() -> void {
    spdlog::info("vengine: run");
    isRunning = true;
    while (isRunning) {
        if (glfwWindowShouldClose(window->get()) == 1) {
            isRunning = false;
            break;
        }
        renderer->render();
    }
}

}  // namespace Vengine