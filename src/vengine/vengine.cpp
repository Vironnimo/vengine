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
        spdlog::error("{}", result.error().message);
    }
}

Vengine::~Vengine() {
    spdlog::debug("Destructor Vengine");
}

[[nodiscard]] auto Vengine::init() -> tl::expected<void, Error> {
    timer = std::make_unique<Timer>();
    timer->start("Vengine.start");

    // spdlog stuff
    spdlog::set_level(spdlog::level::debug);
    spdlog::flush_on(spdlog::level::debug);
    spdlog::info("Vengine: starting...");

    events = std::make_unique<EventSystem>();

    timer->start("Vengine.window_creation");
    window = std::make_shared<Window>();
    if (auto result = window->init(); !result) {
        return tl::unexpected(result.error());
    }
    WindowParams params;
    if (auto result = window->create(params); !result) {
        return tl::unexpected(result.error());
    }
    spdlog::info("Vengine: window creation time: {} ms", timer->stop("Vengine.window_creation"));

    renderer = std::make_unique<Renderer>();
    if (auto result = renderer->init(window); !result) {
        return tl::unexpected(result.error());
    }

    resourceManager = std::make_unique<ResourceManager>();
    if (auto result = resourceManager->init(); !result) {
        return tl::unexpected(result.error());
    }

    actions = std::make_unique<Actions>();
    meshLoader = std::make_unique<MeshLoader>();

    // this is weird here, needs to move
    glfwSetWindowUserPointer(window->get(), this);

    // end timer and print
    auto elapsedTime = timer->stop("Vengine.start");
    spdlog::info("Vengine: initialization took {} ms", elapsedTime);

    spdlog::info("Vengine: successfully started.");
    return {};
}

auto Vengine::run() -> void {
    spdlog::info("Vengine: run");
    isRunning = true;

    while (isRunning) {
        if (glfwWindowShouldClose(window->get()) == 1) {
            isRunning = false;
            break;
        }

        actions->handleInput(window->get());
        renderer->render();
    }
}

}  // namespace Vengine