#include "app.hpp"

#include "vengine/core/model.hpp"
#include "vengine/ecs/components.hpp"
#include "vengine/vengine.hpp"

#include <windows.h>
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

    setupFonts();

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

auto App::setupFonts() -> void {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    auto* defaultFont = io.Fonts->AddFontFromFileTTF("resources/fonts/inter_24_regular.ttf", 17.0f);
    // auto headerFont = io.Fonts->AddFontFromFileTTF("resources/fonts/Roboto-Bold.ttf", 18.0f);

    // ImFontConfig fontConfig;
    // fontConfig.OversampleH = 2;
    // fontConfig.OversampleV = 1;
    // fontConfig.PixelSnapH = true;

    if (!defaultFont) {
        spdlog::warn("Failed to load default font, using ImGui default");
        defaultFont = io.Fonts->AddFontDefault();
    }

    io.Fonts->Build();
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
    ImGui::Text("Delta Time: %.3f ms", static_cast<double>(vengine->timers->deltaTime()));
    ImGui::Text("FPS: %d", fps);

    ImGui::SeparatorText("ECS");
    auto entityCount = vengine->ecs->getEntityCount();
    ImGui::Text("Entities: %zu", entityCount);
    auto systemCount = vengine->ecs->getSystemCount();
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

    // renderer
    ImGui::SeparatorText("Renderer");
    auto drawCalls = vengine->renderer->getDrawCallCount();
    auto vertexCount = vengine->renderer->getVertexCount();
    auto triangleCount = vengine->renderer->getTriangleCount();
    ImGui::Text("Draw Calls: %zu", drawCalls);
    ImGui::Text("Vertices: %zu", vertexCount);
    ImGui::Text("Triangles: %zu", triangleCount);

    // resource manager
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

    // thread Manager
    auto workerCount = vengine->threadManager->getWorkerCount();
    auto activeTasks = vengine->threadManager->getActiveTaskCount();
    auto completedTasks = vengine->threadManager->getCompletedTasks();
    auto completedMainThreadTasks = vengine->threadManager->getCompletedMainThreadTasks();
    ImGui::SeparatorText("Threading");
    ImGui::Text("Workers: %zu", workerCount);
    ImGui::Text("Active Tasks: %zu", activeTasks);
    ImGui::Text("Completed Tasks: %zu", completedTasks);
    ImGui::Text("Completed Main Thread Tasks: %zu", completedMainThreadTasks);

    // scene
    ImGui::SeparatorText("Scene");
    auto currentScene = vengine->scenes->getCurrentScene();
    if (currentScene) {
        ImGui::Text("Current Scene: %s", vengine->getCurrentSceneName().c_str());
    } else {
        ImGui::Text("No active scene");
    }

    // opengl
    ImGui::SeparatorText("OpenGL");
    ImGui::Text("OpenGL Version: %s", glGetString(GL_VERSION));
    ImGui::Text("GPU: %s", glGetString(GL_RENDERER));
    bool vsyncEnabled = vengine->renderer->isVSyncEnabled();
    ImGui::Text("VSync: %s", vsyncEnabled ? "Enabled" : "Disabled");
    bool msaaEnabled = vengine->renderer->isMsaaEnabled();
    ImGui::Text("MSAA: %s", msaaEnabled ? "Enabled" : "Disabled");

    // window
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
        if (auto model = vengine->ecs->getEntityComponent<Vengine::ModelComponent>(entityId)) {
            ImGui::BulletText("Model Component");
        } else {
            std::string loadModelButtonId = "Load Model##" + std::to_string(entityId);
            if (ImGui::Button(loadModelButtonId.c_str())) {
                std::string filter =
                    "3D Model Files\0*.obj;*.fbx;*.dae;*.gltf;*.glb;*.blend;*.3ds;*.ply\0"
                    "OBJ Files\0*.obj\0"
                    "FBX Files\0*.fbx\0"
                    "COLLADA Files\0*.dae\0"
                    "glTF Files\0*.gltf;*.glb\0"
                    "Blender Files\0*.blend\0"
                    "All Files\0*.*\0\0";

                std::string selectedFile = openFileDialog("Select 3D Model", filter);

                if (!selectedFile.empty()) {
                    // Convert absolute path to relative path based on resources directory
                    std::filesystem::path absolutePath(selectedFile);
                    std::filesystem::path resourcesPath = std::filesystem::current_path() / "resources";

                    std::string relativePath;
                    if (absolutePath.string().find(resourcesPath.string()) == 0) {
                        // File is in resources directory, make it relative
                        relativePath = std::filesystem::relative(absolutePath, resourcesPath).string();
                    } else {
                        // File is outside resources, use filename only (assuming it will be copied)
                        relativePath = absolutePath.filename().string();
                        spdlog::warn("Selected file is outside resources directory. Using filename: {}", relativePath);
                    }

                    // Load the model asynchronously
                    std::string modelName = "model_" + std::to_string(entityId);
                    auto defaultShader = vengine->resourceManager->get<Vengine::Shader>("default");

                    try {
                        vengine->resourceManager->loadModelAsync(modelName, relativePath, defaultShader);
                        auto model = vengine->resourceManager->get<Vengine::Model>(modelName);

                        if (model) {
                            vengine->ecs->addComponent<Vengine::ModelComponent>(entityId, model);
                            spdlog::info("Model component added to entity {}", entityId);
                        } else {
                            spdlog::error("Failed to load model for entity {}", entityId);
                        }

                        spdlog::info("Loading model '{}' for entity {}", relativePath, entityId);
                    } catch (const std::exception& e) {
                        spdlog::error("Failed to load model '{}': {}", relativePath, e.what());
                    }
                }
            }
        }
        if (vengine->ecs->getEntityComponent<Vengine::MeshComponent>(entityId)) {
            ImGui::BulletText("Mesh Component");
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

auto App::openFileDialog(const std::string& title, const std::string& filter) -> std::string {
    OPENFILENAME ofn;
    char fileName[MAX_PATH] = "";
    auto defaultPath = std::filesystem::current_path() / "resources";
    std::string defaultPathStr = defaultPath.string();

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filter.c_str();
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = title.c_str();
    ofn.lpstrInitialDir = defaultPathStr.c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileName(&ofn)) {
        return std::string(fileName);
    }

    return ""; 
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
