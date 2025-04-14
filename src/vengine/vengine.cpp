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
    timers = std::make_unique<Timers>();
    timers->start("Vengine.start");

    // spdlog stuff
    spdlog::set_level(spdlog::level::debug);
    spdlog::flush_on(spdlog::level::debug);
    spdlog::info("Vengine: starting...");

    events = std::make_unique<EventSystem>();

    timers->start("Vengine.window_creation");
    window = std::make_shared<Window>();
    if (auto result = window->init(); !result) {
        return tl::unexpected(result.error());
    }
    WindowParams params;
    if (auto result = window->create(params); !result) {
        return tl::unexpected(result.error());
    }
    spdlog::info("Vengine: window creation time: {} ms", timers->stop("Vengine.window_creation"));

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
    auto elapsedTime = timers->stop("Vengine.start");
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

        for (auto& layer : m_layers) {
            layer->onUpdate(timers->deltaTime());
        }

        timers->update();
        actions->handleInput(window->get());
        renderer->render(timers->deltaTime());
    }
}

void Vengine::addLayer(std::shared_ptr<Layer> layer) {
    layer->onAttach();
    m_layers.push_back(std::move(layer));
}

void Vengine::removeLayer(std::shared_ptr<Layer> layer) {
    auto it = std::find(m_layers.begin(), m_layers.end(), layer);
    if (it != m_layers.end()) {
        (*it)->onDetach();
        m_layers.erase(it);
    }
}

}  // namespace Vengine