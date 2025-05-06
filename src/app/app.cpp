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

    // sleep until skybox textures are loaded
    while (!m_vengine->resourceManager->isLoaded("skybox_back") || !m_vengine->resourceManager->isLoaded("skybox_front") ||
           !m_vengine->resourceManager->isLoaded("skybox_left") || !m_vengine->resourceManager->isLoaded("skybox_right") ||
           !m_vengine->resourceManager->isLoaded("skybox_top") || !m_vengine->resourceManager->isLoaded("skybox_bottom")) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // -------------------- ACTIONS ---------------------
    m_vengine->actions->add("quit", "Quit", [this]() { m_vengine->isRunning = false; });
    m_vengine->actions->addKeybinding("quit", {GLFW_KEY_ESCAPE, false, false, false});

    // lets add actions to move the camera
    auto cam = m_vengine->ecs->getEntityByTag("DefaultCamera");
    auto camComp = m_vengine->ecs->getEntityComponent<Vengine::CameraComponent>(cam.getId());

    // zoom in and out with scrollwheel
    m_vengine->actions->add("camera.zoom.in", "Zoom In", [camComp]() { camComp->fov -= 1.0f; });
    m_vengine->actions->addKeybinding("camera.zoom.in", {GLFW_KEY_UP, false, false, false});
    m_vengine->actions->add("camera.zoom.out", "Zoom Out", [camComp]() { camComp->fov += 1.0f; });
    m_vengine->actions->addKeybinding("camera.zoom.out", {GLFW_KEY_DOWN, false, false, false});

    // add scenes
    auto testScene = std::make_shared<TestScene>("TestScene");
    auto testScene2 = std::make_shared<TestScene2>("TestScene2");
    m_vengine->addScene("TestScene", testScene);
    m_vengine->addScene("TestScene2", testScene2);
    m_vengine->loadScene("TestScene");

    // switch scenes (o and p)
    m_vengine->actions->add("scene.switch.scene1", "Switch Scene", [this]() {
        if (m_vengine->getCurrentSceneName() != "TestScene") {
            m_vengine->loadScene("TestScene");
        }
    });
    m_vengine->actions->add("scene.switch.scene2", "Switch Scene", [this]() {
        if (m_vengine->getCurrentSceneName() != "TestScene2") {
            m_vengine->loadScene("TestScene2");
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
    spdlog::info("App fully loaded in {} ms, starting main loop.", end);

    m_vengine->run();
}

int main() {
    App app;
    app.run();

    return 0;
}
