#include "app.hpp"

#include <memory>

// #include "vengine/core/resource_manager.hpp"
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

    m_vengine->resourceManager->load<Vengine::Texture>("test_texture", "test.jpg");
    auto texture = m_vengine->resourceManager->get<Vengine::Texture>("test_texture");

    // create materials
    auto redMaterial = Vengine::Material::createColoredMaterial(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    auto textureMaterial = Vengine::Material::createTexturedMaterial(std::shared_ptr<Vengine::Texture>(texture));

    // add mesh together with material to renderer
    m_vengine->renderer->addRenderObject(triangle, redMaterial);
    m_vengine->renderer->addRenderObject(quad, textureMaterial);

}

void App::run() {
    m_vengine->run();
}

int main() {
    App app;
    app.run();

    return 0;
}
