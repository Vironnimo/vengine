#include "renderer.hpp"

#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <tl/expected.hpp>
#include <utility>

namespace Vengine {

auto Renderer::render() -> void {
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    auto shader = m_shaders->get("shader1");
    if (!shader) {
        spdlog::error("{}", shader.error().message);
        return;
    }
    shader.value()->bind();

    m_mesh_temp->setPosition(glm::vec3(-0.2f, -0.2f, 0.0f));
    shader.value()->setUniformMat4("uTransform", m_mesh_temp->getTransform());
    shader.value()->setUniformVec4("uColor", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));  // red
    m_mesh_temp->draw();

    // m_mesh_temp2->setRotation(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    m_mesh_temp2->setScale(glm::vec3(0.5f, 0.5f, 0.5f));
    m_mesh_temp2->setPosition(glm::vec3(0.2f, 0.2f, 0.0f));
    shader.value()->setUniformMat4("uTransform", m_mesh_temp2->getTransform());
    shader.value()->setUniformVec4("uColor", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)); // blue
    m_mesh_temp2->draw();

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
    std::vector<float> triangleVertices = {
        -0.5f, 0.5f,  0.0f,  // top
        -0.9f, -0.5f, 0.0f,  // bottom left
        -0.1f, -0.5f, 0.0f   // bottom right
    };

    std::vector<float> quadVertices = {
        0.1f, 0.5f,  0.0f,  // top left
        0.9f, 0.5f,  0.0f,  // top right
        0.9f, -0.5f, 0.0f,  // bottom right
        0.1f, -0.5f, 0.0f   // bottom left
    };
    std::vector<uint32_t> quadIndices = {
        0, 1, 2,  // first triangle
        2, 3, 0   // second triangle
    };

    m_mesh_temp = std::make_shared<Mesh>(triangleVertices);
    m_mesh_temp2 = std::make_shared<Mesh>(quadVertices, quadIndices);
    m_shaders = std::make_unique<Shaders>();
    auto shaderInit = m_shaders->init();
    if (!shaderInit) {
        return tl::unexpected(shaderInit.error());
    }

    return {};
}

auto Renderer::setVSync(bool enabled) -> void {
    if (enabled) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
    }
}

}  // namespace Vengine