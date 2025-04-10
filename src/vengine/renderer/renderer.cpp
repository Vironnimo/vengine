#include "renderer.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <tl/expected.hpp>
#include "vengine/core/error.hpp"
#include <utility>
#include "material.hpp"
#include "vengine/renderer/camera.hpp"
#include "vengine/vengine.hpp"

namespace Vengine {

Renderer::Renderer() {
    spdlog::debug("Constructor Renderer");
}

Renderer::~Renderer() {
    spdlog::debug("Destructor Renderer");
}

auto Renderer::render() -> void {
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);  // so closer objects obscure objects further away (i've experienced problems with just one
                              // object)

    // blend is needed for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // actual rendering
    for (const auto& object : m_renderObjects) {
        object.material->bind();

        // NOTE setting all uniforms for each object is bad, lots of redundant calls.. but it works for now
        object.material->getShader()->setUniformMat4("uView", camera->getViewMatrix());
        object.material->getShader()->setUniformMat4("uProjection", camera->getProjectionMatrix());
        object.material->getShader()->setUniformMat4("uTransform", object.mesh->getTransform());

        object.mesh->draw();
    }

    glfwSwapBuffers(m_window->get());
    glfwPollEvents();
}

[[nodiscard]] auto Renderer::init(std::shared_ptr<Window> window) -> tl::expected<void, Error> {
    assert(window->get() != nullptr && "Window is nullptr");
    m_window = std::move(window);

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

    CameraSettings cameraSettings;
    // todo should be somewhere else?
    int w;
    int h;
    glfwGetWindowSize(m_window->get(), &w, &h);
    cameraSettings.aspectRatio = static_cast<float>(w) / static_cast<float>(h);
    camera = std::make_unique<Camera>(cameraSettings);
    camera->setPosition(glm::vec3(0.0f, 0.0f, 5.0f));

    // glfw callbacks
    glfwSetFramebufferSizeCallback(m_window->get(), [](GLFWwindow* wnd, int width, int height) {
        glViewport(0, 0, width, height);
        auto* vengine = static_cast<Vengine*>(glfwGetWindowUserPointer(wnd));
        vengine->renderer->camera->setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    });
    glfwSetScrollCallback(m_window->get(), [](GLFWwindow* wnd, double, double yoffset) {
        yoffset *= 2.0;
        auto* vengine = static_cast<Vengine*>(glfwGetWindowUserPointer(wnd));
        auto fov = vengine->renderer->camera->getFov();
        fov -= static_cast<float>(yoffset);
        if (fov < 1.0f) {
            fov = 1.0f;
        } else if (fov > 45.0f) {
            fov = 45.0f;
        }
        vengine->renderer->camera->setFov(fov);
    });

    setVSync(true);

    return {};
}

auto Renderer::setVSync(bool enabled) -> void {
    if (enabled) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
    }
}

auto Renderer::addRenderObject(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) -> void {
    m_renderObjects.push_back({std::move(mesh), std::move(material)});
}

}  // namespace Vengine