#include "app.hpp"

#include <memory>

#include "vengine/vengine.hpp"
#include "vengine/renderer/mesh.hpp"

App::App() {
    m_vengine = std::make_unique<Vengine::Vengine>();

    // create meshes
    std::vector<float> triangleVertices = {
        -0.5f, 0.5f,  0.0f,  // top
        -0.9f, -0.5f, 0.0f,  // bottom left
        -0.1f, -0.5f, 0.0f   // bottom right
    };
    auto triangle = std::make_shared<Vengine::Mesh>(triangleVertices);

    std::vector<float> quadVertices = {
        // x    y     z     s     t
        0.1f, 0.5f,  0.0f, 0.0f, 1.0f,  // top left
        0.9f, 0.5f,  0.0f, 1.0f, 1.0f,  // top right
        0.9f, -0.5f, 0.0f, 1.0f, 0.0f,  // bottom right
        0.1f, -0.5f, 0.0f, 0.0f, 0.0f   // bottom left
    };
    std::vector<uint32_t> quadIndices = {
        0, 1, 2,  // first triangle
        2, 3, 0   // second triangle
    };
    auto quad = std::make_shared<Vengine::Mesh>(quadVertices, quadIndices);
    quad->setPosition(glm::vec3(0.0f, 0.4f, 0.0f));

    // load textures
    m_vengine->resourceManager->load<Vengine::Texture>("test_texture", "test.jpg");
    auto texture = m_vengine->resourceManager->get<Vengine::Texture>("test_texture");

    // create shaders
    m_vengine->renderer->shaders->add(std::make_shared<Vengine::Shader>("default", "resources/shaders/vertex.glsl", "resources/shaders/fragment.glsl"));
    // m_vengine->renderer->shaders->add(std::make_shared<Vengine::Shader>("colored", "resources/shaders/vertex.glsl", "resources/shaders/fragment.glsl"));
    auto defaultShader = m_vengine->renderer->shaders->get("default");
    if (!defaultShader) {
        spdlog::error(defaultShader.error().message);
        return;
    }

    // create materials (textures + shaders or just shaders)
    m_vengine->renderer->materials->add("default", std::make_shared<Vengine::Material>(defaultShader.value()));
    auto defaultMaterial = m_vengine->renderer->materials->get("default");
    defaultMaterial->setBool("uUseTexture", false);
    defaultMaterial->setVec4("uColor", glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)); 

    m_vengine->renderer->materials->add("default", std::make_shared<Vengine::Material>(defaultShader.value()));
    auto textured = m_vengine->renderer->materials->get("default");
    textured->setBool("uUseTexture", true);
    textured->setTexture("uTexture", std::move(texture));

    // add mesh together with material to renderer
    m_vengine->renderer->addRenderObject(triangle, defaultMaterial);
    m_vengine->renderer->addRenderObject(quad, textured);
}

void App::run() {
    m_vengine->run();
}

int main() {
    App app;
    app.run();

    return 0;
}
