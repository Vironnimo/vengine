#include "app.hpp"

#include "vengine/core/model.hpp"
#include "vengine/ecs/components.hpp"
#include "vengine/vengine.hpp"

#include <memory>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "editor_scene.hpp"
#include "editor_module.hpp"

namespace Veditor {

App::App() {
    m_vengine = std::make_shared<Vengine::Vengine>();
    m_vengine->renderer->setVSync(true);

    auto defaultShader = m_vengine->resourceManager->get<Vengine::Shader>("default");
    m_vengine->resourceManager->loadModelAsync("cube", "box.obj", defaultShader);
    m_vengine->resourceManager->loadAsync<Vengine::Script>("camera", "camera.lua");
    m_vengine->threadManager->waitForCompletion();

    m_vengine->addScene<EditorScene>("Scene1");
    m_vengine->loadScene("Scene1");

    m_vengine->addModule(std::make_shared<Veditor::EditorModule>());
    auto* window = m_vengine->window->get();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // enable keyboard controls
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // enable gamepad controls

    // init opengl for imgui
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    m_vengine->renderer->setPreRenderCallback([]() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    });
    m_vengine->renderer->setPostRenderCallback([this]() {
        gui(m_vengine);
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    });
}

App::~App() {
}

void App::gui(const std::shared_ptr<Vengine::Vengine>& vengine) {
    ImGui::Begin("Vengine Stats");
    static float fpsTimer = 0.0f;
    static int fpsCount = 0;
    static int fps = 0;

    if (fpsTimer >= 1.0f) {
        fpsTimer = 0.0f;
        fps = fpsCount;
        fpsCount = 0;
    } else {
        fpsCount++;
    }
    fpsTimer += vengine->timers->deltaTime();
    ImGui::Text("FPS: %d", fps);
    auto entityCount = vengine->ecs->getEntityCount();
    ImGui::Text("Entities: %zu", entityCount);
    ImGui::End();
}

void App::run() {
    m_vengine->run();
}

}  // namespace Veditor

int main() {
    Veditor::App app;
    app.run();

    return 0;
}
