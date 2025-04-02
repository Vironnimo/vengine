#include "renderer.hpp"

#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <tl/expected.hpp>
#include <utility>

namespace Vengine {

Renderer::Renderer() {
    spdlog::debug("Constructor Renderer");
}

Renderer::~Renderer() {
    spdlog::debug("Destructor Renderer");
}

auto Renderer::render() -> void {
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    auto shader = m_shaders->get("shader1");
    if (!shader) {
        spdlog::error("{}", shader.error().message);
        return;
    }
    shader.value()->bind();

    m_triangle->setPosition(glm::vec3(-0.2f, -0.2f, 0.0f));
    shader.value()->setUniformMat4("uTransform", m_triangle->getTransform());
    shader.value()->setUniformBool("uUseTexture", false);  // IMPORTANT: explicitly disable textures
    shader.value()->setUniformVec4("uColor", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));  // red
    m_triangle->draw();

    m_rectangle->setRotation(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    // m_rectangle->setScale(glm::vec3(0.5f, 0.5f, 0.5f));
    m_rectangle->setPosition(glm::vec3(0.2f, 0.2f, 0.0f));
    shader.value()->setUniformMat4("uTransform", m_rectangle->getTransform());

    // shader.value()->setUniformVec4("uColor", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));  // blue
    if (m_rectangle->hasTexture()) {
        // Set up texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_rectangle->getTexture()->getTextureID());
        shader.value()->setUniformInt("uTexture", 0);  // Use texture unit 0
        shader.value()->setUniformBool("uUseTexture", true);
    } else {
        // Fallback to color if no texture
        shader.value()->setUniformVec4("uColor", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));  // blue
        shader.value()->setUniformBool("uUseTexture", false);
    }

    m_rectangle->draw();

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

    // std::vector<float> quadVertices = {
    //     0.1f, 0.5f,  0.0f,  // top left
    //     0.9f, 0.5f,  0.0f,  // top right
    //     0.9f, -0.5f, 0.0f,  // bottom right
    //     0.1f, -0.5f, 0.0f   // bottom left
    // };
    // std::vector<uint32_t> quadIndices = {
    //     0, 1, 2,  // first triangle
    //     2, 3, 0   // second triangle
    // };

    // texture test
    std::vector<float> quadVertices = {
        // x     y     z     s     t
        0.1f, 0.5f,  0.0f, 0.0f, 1.0f,  // top left
        0.9f, 0.5f,  0.0f, 1.0f, 1.0f,  // top right
        0.9f, -0.5f, 0.0f, 1.0f, 0.0f,  // bottom right
        0.1f, -0.5f, 0.0f, 0.0f, 0.0f   // bottom left
    };
    std::vector<uint32_t> quadIndices = {
        0, 1, 2,  // first triangle
        2, 3, 0   // second triangle
    };

    m_triangle = std::make_shared<Mesh>(triangleVertices);
    m_rectangle = std::make_shared<Mesh>(quadVertices, quadIndices);
    m_shaders = std::make_unique<Shaders>();
    auto shaderInit = m_shaders->init();
    if (!shaderInit) {
        return tl::unexpected(shaderInit.error());
    }

    // texture test
    m_resourceManager = std::make_shared<ResourceManager>();
    m_resourceManager->init();

    m_resourceManager->load<Texture>("test_texture", "test.jpg");
    auto texture = m_resourceManager->get<Texture>("test_texture");

    if (texture) {
        m_rectangle->setTexture(texture);
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