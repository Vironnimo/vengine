#include "renderer.hpp"

#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <tl/expected.hpp>
#include "vengine/core/error.hpp"
#include <utility>
#include "material.hpp"

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
    // glEnable(GL_DEPTH_TEST);  // for 3d depth rendering (needed? for 3d objects?)

    // blend is needed for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // actual rendering
    for (const auto& object : m_renderObjects) {
        object.material->bind();
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

    // glfw callbacks
    glfwSetFramebufferSizeCallback(m_window->get(),
                                   [](GLFWwindow*, int width, int height) { glViewport(0, 0, width, height); });

    // temp stuff
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