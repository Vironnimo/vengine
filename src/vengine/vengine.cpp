#include "vengine.hpp"

#include <spdlog/spdlog.h>

#include <memory>
#include <tl/expected.hpp>

#include "vengine/renderer/renderer.hpp"
// #include "vengine/resource_manager/resource_manager.hpp"

namespace Vengine {

Vengine::Vengine() {
    auto result = init();
    if (!result) {
        spdlog::info("{}", result.error().message);
    }
}

[[nodiscard]] auto Vengine::init() -> tl::expected<void, Error> {
    // tl::expected<void, Error> Vengine::init() {
    m_window = std::make_shared<Window>();
    if (auto result = m_window->init(); !result) {
        return tl::unexpected(result.error());
    }
    WindowParams params;
    if (auto result = m_window->create(params); !result) {
        return tl::unexpected(result.error());
    }

    m_Renderer = std::make_unique<Renderer>();
    if (auto result = m_Renderer->init(m_window); !result) {
        return tl::unexpected(result.error());
    }

    // m_ResourceManager = std::make_unique<ResourceManager>();
    // if (auto result = m_ResourceManager->init(); !result) {
    //     return tl::unexpected(result.error());
    // }

    return {};
}

auto Vengine::run() -> void {
    spdlog::info("vengine: run");
    isRunning = true;
    while (isRunning) {
        if (glfwWindowShouldClose(m_window->get()) == 1) {
            isRunning = false;
            break;
        }
        // while (glfwWindowShouldClose(m_window->get()) == 0) {
        m_Renderer->render();
    }
}

}  // namespace Vengine