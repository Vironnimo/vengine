#include "renderer.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <memory>
#include <tl/expected.hpp>
#include "vengine/core/error.hpp"
#include <utility>
#include "vengine/core/event_system.hpp"
#include "vengine/renderer/fonts.hpp"
#include "vengine/renderer/font.hpp"

namespace Vengine {

Renderer::Renderer() {
    spdlog::debug("Constructor Renderer");
}

Renderer::~Renderer() {
    spdlog::debug("Destructor Renderer");
}

auto Renderer::render(const std::shared_ptr<ECS>& ecs, float deltaTime) -> void {
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);  // so closer objects obscure objects further away (i've experienced problems with just one
                              // object)

    // blend is needed for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // render all viable entities
    if (m_activeCamera == 0) {
        spdlog::error("RenderSystem: No active camera found.");
        // TODO: defaults on error?
        return;
    }

    // camera stuff
    auto cameraTransform = ecs->getEntityComponent<TransformComponent>(m_activeCamera);
    auto cameraComponent = ecs->getEntityComponent<CameraComponent>(m_activeCamera);
    glm::mat4 viewMatrix = cameraComponent->getViewMatrix(*cameraTransform);
    glm::mat4 projectionMatrix = cameraComponent->getProjectionMatrix();

    // render viable entities
    auto list = ecs->getEntitiesWith<TransformComponent, MeshComponent, MaterialComponent>();
    for (auto entity : list) {
        auto transformComp = ecs->getEntityComponent<TransformComponent>(entity);
        auto meshComp = ecs->getEntityComponent<MeshComponent>(entity);
        auto materialComp = ecs->getEntityComponent<MaterialComponent>(entity);

        if (transformComp && meshComp && meshComp->mesh && materialComp && materialComp->material) {
            materialComp->material->bind();

            auto shader = materialComp->material->getShader();
            if (!shader) {
                spdlog::warn("Material has no shader, skipping entity {}", entity);
                continue;
            }

            // TODO: should this be in the material as references?
            shader->setUniformMat4("uView", viewMatrix);
            shader->setUniformMat4("uProjection", projectionMatrix);
            shader->setUniformMat4("uTransform", transformComp->transform);

            meshComp->mesh->draw();
        } else {
            spdlog::warn("Entity {} is missing required components for rendering", entity);
        }
    }

    // TODO: handle the skybox some other way, in shader?
    if (m_skyboxEnabled) {
        skybox->render(viewMatrix, projectionMatrix);
    }

    // TODO: redo text stuff, should become a component
    for (const auto& textObject : m_textObjects) {
        textObject->font->draw(textObject->text, textObject->x, textObject->y, textObject->scale, textObject->color);
    }

    glfwSwapBuffers(m_window->get());
    glfwPollEvents();
}

[[nodiscard]] auto Renderer::init(std::shared_ptr<Window> window, std::shared_ptr<ECS> ecs) -> tl::expected<void, Error> {
    assert(window->get() != nullptr && "Window is nullptr");
    m_window = std::move(window);
    m_ecs = std::move(ecs);

    // NOTE we need the window already opened before calling this
    // TODO move this somewhere, maybe window class i dont know, or just keep it here?
    // NOLINTNEXTLINE(clang-diagnostic-cast-function-type-strict)
    int status = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    if (status == 0) {
        glfwDestroyWindow(m_window->get());
        glfwTerminate();
        return tl::unexpected(Error{"Failed to initialize GLAD"});
    }

    shaders = std::make_unique<Shaders>();
    auto shaderInit = shaders->init();
    if (!shaderInit) {
        return tl::unexpected(shaderInit.error());
    }

    materials = std::make_unique<Materials>();
    auto materialsInit = materials->init();
    if (!materialsInit) {
        return tl::unexpected(materialsInit.error());
    }

    fonts = std::make_unique<Fonts>();
    if (!fonts->init(shaders->get("default.text").value())) {
        return tl::unexpected(Error{"Failed to initialize fonts"});
    }

    // should also be somewhere else
    setVSync(false);

    return {};
}

[[nodiscard]] auto Renderer::loadSkybox(const std::vector<std::shared_ptr<Texture>>& faceFiles) -> bool {
    shaders->add(std::make_shared<Shader>("skybox", "resources/shaders/skybox.vert", "resources/shaders/skybox.frag"));
    skybox = std::make_unique<Skybox>();
    skybox->setShader(shaders->get("skybox").value());

    if (skybox->loadFromTextures(faceFiles)) {
        m_skyboxEnabled = true;
        return true;
    }

    m_skyboxEnabled = false;
    return false;
}

auto Renderer::unloadSkybox() -> void {
    if (skybox) {
        skybox->unload();
        m_skyboxEnabled = false;
    } else {
        spdlog::warn("Skybox not loaded, cannot unload");
    }
}

auto Renderer::setVSync(bool enabled) -> void {
    if (enabled) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
    }
}

auto Renderer::addTextObject(std::shared_ptr<TextObject> textObject) -> void {
    m_textObjects.push_back(std::move(textObject));
}

// TODO: what to do with all the glfw callbacks?
auto Renderer::setActiveCamera(EntityId camera) -> void {
    if (m_activeCamera != 0 && m_activeCamera != camera) {
        auto oldCamComp = m_ecs->getActiveEntities()->getEntityComponent<CameraComponent>(m_activeCamera);
        if (oldCamComp) {
            oldCamComp->isActive = false;
        }
    }

    auto camComp = m_ecs->getActiveEntities()->getEntityComponent<CameraComponent>(camera);
    if (!camComp) {
        // TODO: save old camera id and reset. and what happens if the old camera is already destroyed?
        spdlog::error("Renderer: Cannot set active camera {}. Missing CameraComponent.", camera);
        glfwSetScrollCallback(m_window->get(), nullptr);
        m_activeCamera = 0;
        return;
    }

    m_activeCamera = camera;
    camComp->isActive = true;
    m_ecs->setActiveCamera(camera);

    spdlog::info("Renderer: Set active camera to {}", camera);
    g_eventSystem.publish(CameraChangedEvent{camera});
}

}  // namespace Vengine