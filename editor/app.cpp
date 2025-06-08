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
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // init opengl for imgui
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    m_vengine->renderer->setPreRenderCallback([]() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    });
    m_vengine->renderer->setPostRenderCallback([this]() {
        statsPanel(m_vengine);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    });
}

App::~App() {
}

void App::statsPanel(const std::shared_ptr<Vengine::Vengine>& vengine) {
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
    ImGui::Text("Delta Time: %.3f ms", vengine->timers->deltaTime());
    ImGui::Text("FPS: %d", fps);

    ImGui::SeparatorText("ECS");
    auto entityCount = vengine->ecs->getEntityCount();
    ImGui::Text("Entities: %zu", entityCount);
    auto systemCount = vengine->ecs->getSystemCount();  // If you have this method
    ImGui::Text("Registered Systems: %zu", systemCount);
    std::string nodeText = "Registered Components: " + std::to_string(vengine->ecs->getComponentCount());
    if (ImGui::TreeNode(nodeText.c_str())) {
        auto transformEntities = vengine->ecs->getEntitiesWith<Vengine::TransformComponent>();
        auto meshEntities = vengine->ecs->getEntitiesWith<Vengine::MeshComponent>();
        auto modelEntities = vengine->ecs->getEntitiesWith<Vengine::ModelComponent>();
        auto cameraEntities = vengine->ecs->getEntitiesWith<Vengine::CameraComponent>();
        auto lightEntities = vengine->ecs->getEntitiesWith<Vengine::LightComponent>();
        auto scriptEntities = vengine->ecs->getEntitiesWith<Vengine::ScriptComponent>();
        auto physicsEntities = vengine->ecs->getEntitiesWith<Vengine::PhysicsComponent>();
        ImGui::Text("Transform Components: %zu", transformEntities.size());
        ImGui::Text("Mesh Components: %zu", meshEntities.size());
        ImGui::Text("Model Components: %zu", modelEntities.size());
        ImGui::Text("Camera Components: %zu", cameraEntities.size());
        ImGui::Text("Light Components: %zu", lightEntities.size());
        ImGui::Text("Script Components: %zu", scriptEntities.size());
        ImGui::Text("Physics Components: %zu", physicsEntities.size());
        ImGui::TreePop();
    }

    // renderer Stats
    ImGui::SeparatorText("Renderer");
    auto drawCalls = vengine->renderer->getDrawCallCount();
    auto vertexCount = vengine->renderer->getVertexCount();
    auto triangleCount = vengine->renderer->getTriangleCount();
    ImGui::Text("Draw Calls: %zu", drawCalls);
    ImGui::Text("Vertices: %zu", vertexCount);
    ImGui::Text("Triangles: %zu", triangleCount);

    // resource Manager Stats
    ImGui::SeparatorText("Resources");
    auto textureCount = vengine->resourceManager->getLoadedCount<Vengine::Texture>();
    auto meshCount = vengine->resourceManager->getLoadedCount<Vengine::Mesh>();
    auto modelCount = vengine->resourceManager->getLoadedCount<Vengine::Model>();
    auto shaderCount = vengine->resourceManager->getLoadedCount<Vengine::Shader>();
    auto scriptCount = vengine->resourceManager->getLoadedCount<Vengine::Script>();
    auto soundCount = vengine->resourceManager->getLoadedCount<Vengine::Sound>();
    ImGui::Text("Textures: %zu", textureCount);
    ImGui::Text("Meshes: %zu", meshCount);
    ImGui::Text("Models: %zu", modelCount);
    ImGui::Text("Shaders: %zu", shaderCount);
    ImGui::Text("Scripts: %zu", scriptCount);
    ImGui::Text("Sounds: %zu", soundCount);

    // thread Manager Stats
    auto workerCount = vengine->threadManager->getWorkerCount();
    auto activeTasks = vengine->threadManager->getActiveTaskCount();
    auto completedTasks = vengine->threadManager->getCompletedTasks();
    auto completedMainThreadTasks = vengine->threadManager->getCompletedMainThreadTasks();
    ImGui::SeparatorText("Threading");
    ImGui::Text("Workers: %zu", workerCount);
    ImGui::Text("Active Tasks: %zu", activeTasks);
    ImGui::Text("Completed Tasks: %zu", completedTasks);
    ImGui::Text("Completed Main Thread Tasks: %zu", completedMainThreadTasks);

    // Scene Stats
    ImGui::SeparatorText("Scene");
    auto currentScene = vengine->scenes->getCurrentScene();
    if (currentScene) {
        ImGui::Text("Current Scene: %s", vengine->getCurrentSceneName().c_str());
        // Add scene-specific stats here
    } else {
        ImGui::Text("No active scene");
    }

    // OpenGL Stats (if available)
    ImGui::SeparatorText("OpenGL");
    ImGui::Text("OpenGL Version: %s", glGetString(GL_VERSION));
    ImGui::Text("GPU: %s", glGetString(GL_RENDERER));

    // VSync status
    bool vsyncEnabled = vengine->renderer->isVSyncEnabled();
    ImGui::Text("VSync: %s", vsyncEnabled ? "Enabled" : "Disabled");
    bool msaaEnabled = vengine->renderer->isMsaaEnabled();
    ImGui::Text("MSAA: %s", msaaEnabled ? "Enabled" : "Disabled");

    // Window Stats
    ImGui::SeparatorText("Window");
    int windowWidth = vengine->window->getWidth();
    int windowHeight = vengine->window->getHeight();
    ImGui::Text("Resolution: %dx%d", windowWidth, windowHeight);

    ImGui::End();

    entitiesPanel(vengine);
}

auto App::entitiesPanel(const std::shared_ptr<Vengine::Vengine>& vengine) -> void {
    ImGui::Begin("Entity Inspector");

    if (ImGui::Button("Create Entity")) {
        auto newEntity = vengine->ecs->createEntity();
        vengine->ecs->addComponent<Vengine::TagComponent>(newEntity, "New Entity");
        vengine->ecs->addComponent<Vengine::TransformComponent>(newEntity);
    }
    ImGui::Separator();

    auto taggedEntities = vengine->ecs->getEntitiesWith<Vengine::TagComponent>();
    if (taggedEntities.empty()) {
        ImGui::Text("No entities found");
        ImGui::End();
        return;
    }

    ImGui::Text("Entities (%zu):", taggedEntities.size());
    ImGui::Separator();

    for (auto entityId : taggedEntities) {
        entityNode(vengine, entityId);
    }

    ImGui::End();
}

auto App::entityNode(const std::shared_ptr<Vengine::Vengine>& vengine, Vengine::EntityId entityId) -> void {
    auto tagComponent = vengine->ecs->getEntityComponent<Vengine::TagComponent>(entityId);
    std::string entityName = tagComponent ? tagComponent->tag : "Unnamed Entity";

    // tree node
    std::string nodeId = entityName + "##" + std::to_string(entityId);
    if (ImGui::TreeNode(nodeId.c_str())) {
        ImGui::Text("Entity ID: %llu", entityId);
        ImGui::SameLine();
        std::string deleteButtonId = "Delete##" + std::to_string(entityId);
        if (ImGui::Button(deleteButtonId.c_str())) {
            vengine->ecs->destroyEntity(entityId);
            ImGui::TreePop();
            return;  // return to avoid accessing destroyed entity
        }

        ImGui::Separator();

        auto transformComponent = vengine->ecs->getEntityComponent<Vengine::TransformComponent>(entityId);
        if (transformComponent) {
            if (ImGui::CollapsingHeader("Transform")) {
                auto position = transformComponent->getPosition();
                auto rotation = transformComponent->getRotation();
                auto scale = transformComponent->getScale();

                ImGui::Text("Position:");
                std::string posId = "##pos" + std::to_string(entityId);
                if (ImGui::SliderFloat3(("X Y Z" + posId).c_str(), &position.x, -50.0f, 50.0f, "%.2f")) {
                    transformComponent->setPosition(position);
                }

                ImGui::Text("Rotation (degrees):");
                glm::vec3 rotationDegrees = glm::degrees(rotation);
                std::string rotId = "##rot" + std::to_string(entityId);
                if (ImGui::SliderFloat3(("X Y Z" + rotId).c_str(), &rotationDegrees.x, -180.0f, 180.0f, "%.1f")) {
                    transformComponent->setRotation(glm::radians(rotationDegrees));
                }

                ImGui::Text("Scale:");
                std::string scaleId = "##scale" + std::to_string(entityId);
                if (ImGui::SliderFloat3(("X Y Z" + scaleId).c_str(), &scale.x, 0.1f, 5.0f, "%.2f")) {
                    transformComponent->setScale(scale);
                }

                if (ImGui::Button(("Reset Position##" + std::to_string(entityId)).c_str())) {
                    transformComponent->setPosition(glm::vec3(0.0f));
                }
                ImGui::SameLine();
                if (ImGui::Button(("Reset Rotation##" + std::to_string(entityId)).c_str())) {
                    transformComponent->setRotation(glm::vec3(0.0f));
                }
                ImGui::SameLine();
                if (ImGui::Button(("Reset Scale##" + std::to_string(entityId)).c_str())) {
                    transformComponent->setScale(glm::vec3(1.0f));
                }
            }
        }

        ImGui::Separator();
        ImGui::Text("Other Components:");
        if (vengine->ecs->getEntityComponent<Vengine::MeshComponent>(entityId)) {
            ImGui::BulletText("Mesh Component");
        }
        if (vengine->ecs->getEntityComponent<Vengine::ModelComponent>(entityId)) {
            ImGui::BulletText("Model Component");
        }
        if (vengine->ecs->getEntityComponent<Vengine::MaterialComponent>(entityId)) {
            ImGui::BulletText("Material Component");
        }
        if (vengine->ecs->getEntityComponent<Vengine::CameraComponent>(entityId)) {
            ImGui::BulletText("Camera Component");
        }
        if (vengine->ecs->getEntityComponent<Vengine::LightComponent>(entityId)) {
            ImGui::BulletText("Light Component");
        }
        if (vengine->ecs->getEntityComponent<Vengine::ScriptComponent>(entityId)) {
            ImGui::BulletText("Script Component");
        }
        if (vengine->ecs->getEntityComponent<Vengine::PhysicsComponent>(entityId)) {
            ImGui::BulletText("Physics Component");
        }

        ImGui::TreePop();
    }
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
