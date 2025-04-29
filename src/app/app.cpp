#include "app.hpp"
#include <GLFW/glfw3.h>

#include <memory>

#include "app/test_scene.hpp"
#include "app/test_scene2.hpp"
#include "vengine/vengine.hpp"

App::App() {
    m_vengine = std::make_shared<Vengine::Vengine>();
    m_vengine->timers->start("app_constructor");

    // load resources async at the beginning
    m_vengine->resourceManager->loadAsync<Vengine::Texture>("test_texture", "test.jpg");
    m_vengine->resourceManager->loadAsync<Vengine::Texture>("test_texture2", "test2.jpg");
    m_vengine->resourceManager->loadAsync<Vengine::Texture>("skybox_right", "skybox/cube_right.png");
    m_vengine->resourceManager->loadAsync<Vengine::Texture>("skybox_left", "skybox/cube_left.png");
    m_vengine->resourceManager->loadAsync<Vengine::Texture>("skybox_top", "skybox/cube_up.png");
    m_vengine->resourceManager->loadAsync<Vengine::Texture>("skybox_bottom", "skybox/cube_down.png");
    m_vengine->resourceManager->loadAsync<Vengine::Texture>("skybox_back", "skybox/cube_back.png");
    m_vengine->resourceManager->loadAsync<Vengine::Texture>("skybox_front", "skybox/cube_front.png");

    // load sounds
    m_vengine->resourceManager->loadAsync<Vengine::Sound>("click", "click.wav");

    // load fonts
    auto fonts = m_vengine->renderer->fonts->load("default", "inter_24_regular.ttf", 24);
    if (!fonts) {
        spdlog::error(fonts.error().message);
    }

    // load shaders
    m_vengine->renderer->shaders->add(
        std::make_shared<Vengine::Shader>("default", "resources/shaders/default_new.vert", "resources/shaders/default_new.frag"));
    auto defaultShader = m_vengine->renderer->shaders->get("default");
    if (!defaultShader) {
        spdlog::error(defaultShader.error().message);
        return;
    }
    
    // sleep until test_texture is loaded
    while (!m_vengine->resourceManager->isLoaded("test_texture") || !m_vengine->resourceManager->isLoaded("skybox_front")) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    m_vengine->renderer->camera->setPosition(glm::vec3(0.0f, 20.0f, 205.0f));
    // -------------------- ACTIONS ---------------------
    m_vengine->actions->add("quit", "Quit", [this]() { m_vengine->isRunning = false; });
    m_vengine->actions->addKeybinding("quit", {GLFW_KEY_ESCAPE, false, false, false});

    // lets add actions to move the camera
    const float cameraSpeed = 100.0f;
    m_vengine->actions->add("camera.move.right", "Move Camera", [this, cameraSpeed]() {
        m_vengine->renderer->camera->setPosition(m_vengine->renderer->camera->getPosition() +
                                                 glm::vec3(cameraSpeed * m_vengine->timers->deltaTime(), 0.0f, 0.0f));
    });
    m_vengine->actions->addKeybinding("camera.move.right", {GLFW_KEY_F, false, false, false});

    m_vengine->actions->add("camera.move.left", "Move Camera", [this, cameraSpeed]() {
        m_vengine->renderer->camera->setPosition(m_vengine->renderer->camera->getPosition() +
                                                 glm::vec3(-cameraSpeed * m_vengine->timers->deltaTime(), 0.0f, 0.0f));
    });
    m_vengine->actions->addKeybinding("camera.move.left", {GLFW_KEY_S, false, false, false});

    m_vengine->actions->add("camera.move.up", "Move Camera", [this, cameraSpeed]() {
        m_vengine->renderer->camera->setPosition(m_vengine->renderer->camera->getPosition() +
                                                 glm::vec3(0.0f, cameraSpeed * m_vengine->timers->deltaTime(), 0.0f));
    });
    m_vengine->actions->addKeybinding("camera.move.up", {GLFW_KEY_BACKSPACE, false, false, false});

    m_vengine->actions->add("camera.move.down", "Move Camera", [this, cameraSpeed]() {
        m_vengine->renderer->camera->setPosition(m_vengine->renderer->camera->getPosition() +
                                                 glm::vec3(0.0f, -cameraSpeed * m_vengine->timers->deltaTime(), 0.0f));
    });
    m_vengine->actions->addKeybinding("camera.move.down", {GLFW_KEY_DELETE, false, false, false});

    m_vengine->actions->add("camera.move.forward", "Move Camera", [this, cameraSpeed]() {
        m_vengine->renderer->camera->setPosition(m_vengine->renderer->camera->getPosition() +
                                                 glm::vec3(0.0f, 0.0f, -cameraSpeed * m_vengine->timers->deltaTime()));
    });
    m_vengine->actions->addKeybinding("camera.move.forward", {GLFW_KEY_E, false, false, false});

    m_vengine->actions->add("camera.move.backward", "Move Camera", [this, cameraSpeed]() {
        m_vengine->renderer->camera->setPosition(m_vengine->renderer->camera->getPosition() +
                                                 glm::vec3(0.0f, 0.0f, cameraSpeed * m_vengine->timers->deltaTime()));
    });
    m_vengine->actions->addKeybinding("camera.move.backward", {GLFW_KEY_D, false, false, false});

    // zoom in and out with scrollwheel
    m_vengine->actions->add("camera.zoom.in", "Zoom In",
                            [this]() { m_vengine->renderer->camera->setFov(m_vengine->renderer->camera->getFov() - 1.0f); });
    m_vengine->actions->addKeybinding("camera.zoom.in", {GLFW_KEY_UP, false, false, false});
    m_vengine->actions->add("camera.zoom.out", "Zoom Out",
                            [this]() { m_vengine->renderer->camera->setFov(m_vengine->renderer->camera->getFov() + 1.0f); });
    m_vengine->actions->addKeybinding("camera.zoom.out", {GLFW_KEY_DOWN, false, false, false});

    // this is some weird turning with the mouse..
    // we might need/want a mouse class
    m_vengine->actions->add("camera.rotate", "Turn Camera", [this]() {
        double xpos;
        double ypos;
        glfwGetCursorPos(m_vengine->window->get(), &xpos, &ypos);
        // glfwSetCursorPos(m_vengine->window->get(), 1024 / 2, 768 / 2);

        static double lastX = xpos;
        static double lastY = ypos;
        auto xoffset = static_cast<float>(xpos - lastX);
        auto yoffset = static_cast<float>(lastY - ypos);
        yoffset = -yoffset;
        lastX = xpos;
        lastY = ypos;

        m_vengine->renderer->camera->setRotation(m_vengine->renderer->camera->getRotation() +
                                                 glm::vec3(yoffset * 0.01f, xoffset * 0.01f, 0.0f));
    });
    m_vengine->actions->addKeybinding("camera.rotate", {GLFW_MOUSE_BUTTON_LEFT, false, false, false});

    // add scenes
    auto testScene = std::make_shared<TestScene>("TestScene");
    auto testScene2 = std::make_shared<TestScene2>("TestScene2");
    m_vengine->addScene("TestScene", testScene);
    m_vengine->addScene("TestScene2", testScene2);
    m_vengine->switchToScene("TestScene");
    
    // switch scenes (o and p)
    m_vengine->actions->add("scene.switch.scene1", "Switch Scene", [this]() {
        if (m_vengine->getCurrentSceneName() != "TestScene") {
            m_vengine->switchToScene("TestScene");
        }
    });
    m_vengine->actions->add("scene.switch.scene2", "Switch Scene", [this]() {
        if (m_vengine->getCurrentSceneName() != "TestScene2") {
            m_vengine->switchToScene("TestScene2");
        }
    });
    m_vengine->actions->addKeybinding("scene.switch.scene1", {GLFW_KEY_O, false, false, false});
    m_vengine->actions->addKeybinding("scene.switch.scene2", {GLFW_KEY_P, false, false, false});

    // add modules
    m_testModule = std::make_shared<TestModule>();
    m_vengine->addModule(m_testModule);

    auto end = m_vengine->timers->stop("app_constructor");
    spdlog::info("App constructor took {} ms", end);
}

void App::run() {
    auto end = m_vengine->timers->stop("vengine.start");
    spdlog::info("Full App initialization took {} ms", end);

    m_vengine->run();
}

int main() {
    App app;
    app.run();

    return 0;
}
