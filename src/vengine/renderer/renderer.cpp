#include "renderer.hpp"

#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <tl/expected.hpp>
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
    // glEnable(GL_DEPTH_TEST);  // Enable depth testing for 3D rendering (needed? for 3d objects maybe?)


    // blend is needed for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render all objects
    for (const auto& object : m_renderObjects) {
        object.material->bind();
        object.material->getShader()->setUniformMat4("uTransform", object.mesh->getTransform());

        object.mesh->draw();
    }

    // if (m_rectangle->hasTexture()) {
    //     // Set up texture
    //     glActiveTexture(GL_TEXTURE0);
    //     glBindTexture(GL_TEXTURE_2D, m_rectangle->getTexture()->getTextureID());
    //     shader.value()->setUniformInt("uTexture", 0);  // Use texture unit 0
    //     shader.value()->setUniformBool("uUseTexture", true);
    // } else {
    //     // Fallback to color if no texture
    //     shader.value()->setUniformVec4("uColor", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));  // blue
    //     shader.value()->setUniformBool("uUseTexture", false);
    // }
    // m_rectangle->draw();

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
    // m_triangle = std::make_shared<Mesh>(triangleVertices);
    // m_rectangle = std::make_shared<Mesh>(quadVertices, quadIndices);
    m_shaders = std::make_unique<Shaders>();
    auto shaderInit = m_shaders->init();
    if (!shaderInit) {
        return tl::unexpected(shaderInit.error());
    }

    // // texture test
    // m_resourceManager = std::make_shared<ResourceManager>();
    // m_resourceManager->init();

    // m_resourceManager->load<Texture>("test_texture", "test.jpg");
    // auto texture = m_resourceManager->get<Texture>("test_texture");

    // if (texture) {
    //     m_rectangle->setTexture(texture);
    // }

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