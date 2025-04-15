#include "app.hpp"

#include <memory>

#include "vengine/vengine.hpp"
#include "vengine/renderer/renderer.hpp"

App::App() {
    m_vengine = std::make_shared<Vengine::Vengine>();
    m_vengine->timers->start("app_constructor");

    // actions
    m_vengine->actions->add("quit", "Quit", [this]() { m_vengine->isRunning = false; });
    m_vengine->actions->addKeybinding("quit", {GLFW_KEY_ESCAPE, false, false, false});

    // lets add actions to move the camera
    m_vengine->actions->add("move_camera_right", "Move Camera", [this]() {
        m_vengine->renderer->camera->setPosition(m_vengine->renderer->camera->getPosition() +
                                                 glm::vec3(0.05f, 0.0f, 0.0f));
    });
    m_vengine->actions->addKeybinding("move_camera_right", {GLFW_KEY_F, false, false, false});
    m_vengine->actions->add("move_camera_left", "Move Camera", [this]() {
        m_vengine->renderer->camera->setPosition(m_vengine->renderer->camera->getPosition() +
                                                 glm::vec3(-0.05f, 0.0f, 0.0f));
    });
    m_vengine->actions->addKeybinding("move_camera_left", {GLFW_KEY_S, false, false, false});
    m_vengine->actions->add("move_camera_up", "Move Camera", [this]() {
        m_vengine->renderer->camera->setPosition(m_vengine->renderer->camera->getPosition() +
                                                 glm::vec3(0.0f, 0.05f, 0.0f));
    });
    m_vengine->actions->addKeybinding("move_camera_up", {GLFW_KEY_E, false, false, false});
    m_vengine->actions->add("move_camera_down", "Move Camera", [this]() {
        m_vengine->renderer->camera->setPosition(m_vengine->renderer->camera->getPosition() +
                                                 glm::vec3(0.0f, -0.05f, 0.0f));
    });
    m_vengine->actions->addKeybinding("move_camera_down", {GLFW_KEY_D, false, false, false});

    // zoom in and out with scrollwheel
    m_vengine->actions->add("zoom_in", "Zoom In", [this]() {
        m_vengine->renderer->camera->setFov(m_vengine->renderer->camera->getFov() - 1.0f);
    });
    m_vengine->actions->addKeybinding("zoom_in", {GLFW_KEY_UP, false, false, false});
    m_vengine->actions->add("zoom_out", "Zoom Out", [this]() {
        m_vengine->renderer->camera->setFov(m_vengine->renderer->camera->getFov() + 1.0f);
    });
    m_vengine->actions->addKeybinding("zoom_out", {GLFW_KEY_DOWN, false, false, false});

    // this is some weird turning with the mouse..
    // we might need/want a mouse class
    m_vengine->actions->add("turn_camera", "Turn Camera", [this]() {
        double xpos;
        double ypos;
        glfwGetCursorPos(m_vengine->window->get(), &xpos, &ypos);

        static double lastX = xpos;
        static double lastY = ypos;
        float xoffset = static_cast<float>(xpos - lastX);
        float yoffset = static_cast<float>(lastY - ypos);
        yoffset = -yoffset;
        lastX = xpos;
        lastY = ypos;

        m_vengine->renderer->camera->setRotation(m_vengine->renderer->camera->getRotation() +
                                                 glm::vec3(yoffset * 0.01f, xoffset * 0.01f, 0.0f));
    });
    m_vengine->actions->addKeybinding("turn_camera", {GLFW_MOUSE_BUTTON_LEFT, false, false, false});

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

    // 3d cube
    auto cube = m_vengine->meshLoader->loadFromObj("box.obj");
    cube->setScale(glm::vec3(0.5f, 0.5f, 0.5f));
    cube->setRotation(60.0f, glm::vec3(0.0f, 1.0f, 1.0f));

    // load textures
    m_vengine->resourceManager->load<Vengine::Texture>("test_texture", "test.jpg");
    auto texture = m_vengine->resourceManager->get<Vengine::Texture>("test_texture");

    // create shaders
    m_vengine->renderer->shaders->add(std::make_shared<Vengine::Shader>("default", "resources/shaders/default.vert",
                                                                        "resources/shaders/default.frag"));
    // m_vengine->renderer->shaders->add(std::make_shared<Vengine::Shader>("colored", "resources/shaders/vertex.glsl",
    // "resources/shaders/fragment.glsl"));
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
    m_vengine->renderer->addRenderObject(cube, textured);

    // load font
    auto fonts = m_vengine->renderer->fonts->load("default", "inter_24_regular.ttf", 24);
    if (!fonts) {
        spdlog::error(fonts.error().message);
    }

    // skybox
    // order matters here! right, left, top, bottom, back, front
    m_vengine->timers->start("skybox");
    std::vector<std::string> skyboxFaces = {
        "resources/textures/skybox/cube_right.png", "resources/textures/skybox/cube_left.png",
        "resources/textures/skybox/cube_up.png",    "resources/textures/skybox/cube_down.png",
        "resources/textures/skybox/cube_back.png",  "resources/textures/skybox/cube_front.png"
    };

    if (!m_vengine->renderer->loadSkybox(skyboxFaces)) {
        spdlog::error("Failed to load skybox textures");
    }
    auto time = m_vengine->timers->stop("skybox");
    spdlog::info("Skybox loaded in {} ms", time);

    // layers
    m_testLayer = std::make_shared<TestLayer>(m_vengine);
    m_vengine->addLayer(m_testLayer);

    auto end = m_vengine->timers->stop("app_constructor");
    spdlog::info("App constructor took {} ms", end);
}

void App::run() {
    auto end = m_vengine->timers->stop("Vengine.start");
    spdlog::info("Full App initialization took {} ms", end);
    m_vengine->run();
}

int main() {
    App app;
    app.run();

    return 0;
}
