#include "renderer.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <memory>
#include <tl/expected.hpp>
#include "vengine/core/error.hpp"
#include <utility>
#include "vengine/renderer/fonts.hpp"
#include "vengine/renderer/font.hpp"
#include "vengine/ecs/systems.hpp"

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
    auto renderSystem = ecs->getSystem<RenderSystem>("RenderSystem");
    if (renderSystem) {
        renderSystem->update(ecs->getActiveEntities(), deltaTime);
    } else {
        spdlog::warn("RenderSystem not found in ECS");
    }

    // TODO: copied here from renderSystem, needs to be gone, just here for the skybox which changes anyway
    EntityId activeCameraId = 0;
    auto cameraEntities = ecs->getActiveEntities()->getEntitiesWith<CameraComponent, TransformComponent>();
    for (auto camId : cameraEntities) {
        auto camComp = ecs->getActiveEntities()->getEntityComponent<CameraComponent>(camId);
        if (camComp && camComp->isActive) {
            activeCameraId = camId;
            break;
        }
    }

    if (activeCameraId == 0) {
        spdlog::error("RenderSystem: No active camera found.");
        return;  // Or render with default matrices?
    }
    auto cameraTransform = ecs->getActiveEntities()->getEntityComponent<TransformComponent>(activeCameraId);
    auto cameraComponent = ecs->getActiveEntities()->getEntityComponent<CameraComponent>(activeCameraId);

    glm::mat4 camRotationMat = glm::rotate(glm::mat4(1.0f), cameraTransform->rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
                               glm::rotate(glm::mat4(1.0f), cameraTransform->rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
                               glm::rotate(glm::mat4(1.0f), cameraTransform->rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 camTranslationMat = glm::translate(glm::mat4(1.0f), -cameraTransform->position);
    glm::mat4 viewMatrix = camRotationMat * camTranslationMat;  // Common view matrix calculation
    glm::mat4 projectionMatrix = cameraComponent->getProjectionMatrix();

    if (m_skyboxEnabled) {
        skybox->render(viewMatrix, projectionMatrix);
    }

    // render text objects
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

    // glfw callbacks
    // user pointer
    glfwSetWindowUserPointer(m_window->get(), this);

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
    m_activeCamera = camera;

    glfwSetScrollCallback(m_window->get(), [](GLFWwindow* wnd, double, double yoffset) {
        yoffset *= 2.0;  // sens
        auto* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(wnd));
        if (!renderer || renderer->m_activeCamera == 0) {
            return;
        }
        auto cameraTransform = renderer->m_ecs->getEntityComponent<TransformComponent>(renderer->m_activeCamera);
        auto camComp = renderer->m_ecs->getEntityComponent<CameraComponent>(renderer->m_activeCamera);
        if (camComp && cameraTransform) {
            camComp->fov -= static_cast<float>(yoffset);
            // TODO: max fov somewhere else? and not hardcoded...
            if (camComp->fov < 1.0f) {
                camComp->fov = 1.0f;
            }
            if (camComp->fov > 90.0f) {
                camComp->fov = 90.0f;
            }
        }
    });

    glfwSetFramebufferSizeCallback(m_window->get(), [](GLFWwindow* wnd, int width, int height) {
        glViewport(0, 0, width, height);
        auto* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(wnd)); 
        if (renderer && renderer->m_ecs) {
            auto entities = renderer->m_ecs->getActiveEntities();
            auto cameraEntities = entities->getEntitiesWith<CameraComponent>(); 
            for (auto camId : cameraEntities) {
                auto camComp = entities->getEntityComponent<CameraComponent>(camId);
                if (camComp && camComp->isActive) {  
                    camComp->aspectRatio = static_cast<float>(width) / static_cast<float>(height);
                    break;  
                }
            }
        }
    });
}

}  // namespace Vengine