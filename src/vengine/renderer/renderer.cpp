#include "renderer.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <memory>
#include <tl/expected.hpp>
#include "vengine/core/error.hpp"
#include <utility>
#include "vengine/renderer/fonts.hpp"
#include "vengine/renderer/font.hpp"

namespace Vengine {

// test stuff
struct MeshMaterialKey {
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
    bool operator<(const MeshMaterialKey& other) const {
        return std::tie(mesh, material) < std::tie(other.mesh, other.material);
    }
};

struct MeshSubmeshMaterialKey {
    std::shared_ptr<Mesh> mesh;
    size_t submeshIndex;
    std::shared_ptr<Material> material;
    bool operator<(const MeshSubmeshMaterialKey& other) const {
        return std::tie(mesh, submeshIndex, material) < std::tie(other.mesh, other.submeshIndex, other.material);
    }
};

static auto uploadInstanceTransforms(const std::shared_ptr<Mesh>& mesh, const std::vector<glm::mat4>& transforms) -> void {
    static std::unordered_map<std::shared_ptr<Mesh>, GLuint> instanceVBOs;
    GLuint instanceVBO = 0;
    if (instanceVBOs.find(mesh) == instanceVBOs.end()) {
        glGenBuffers(1, &instanceVBO);
        instanceVBOs[mesh] = instanceVBO;
    } else {
        instanceVBO = instanceVBOs[mesh];
    }

    glBindVertexArray(mesh->getVertexArray()->getID());
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4), transforms.data(), GL_DYNAMIC_DRAW);

    // Set up 4 vec4 attributes for mat4 (locations 4,5,6,7)
    for (int i = 0; i < 4; ++i) {
        glEnableVertexAttribArray(4 + i);
        glVertexAttribPointer(4 + i,
                              4,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(glm::mat4),
                              reinterpret_cast<void*>(sizeof(float) * static_cast<size_t>(i) * 4));
        glVertexAttribDivisor(4 + i, 1);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
// end test stuff

Renderer::Renderer() {
    spdlog::debug("Constructor Renderer");
}

Renderer::~Renderer() {
    spdlog::debug("Destructor Renderer");
}

auto Renderer::render(const std::shared_ptr<Scene>& scene) -> void {
    glClearColor(0.4f, 0.6f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    // blend is needed for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (scene->getCameras()->getActive() == 0) {
        spdlog::error("RenderSystem: No active camera found.");
        // TODO: defaults on error?
        return;
    }

    // camera stuff
    auto cameraTransform = scene->getEntities()->getEntityComponent<TransformComponent>(scene->getCameras()->getActive());
    auto cameraComponent = scene->getEntities()->getEntityComponent<CameraComponent>(scene->getCameras()->getActive());
    glm::mat4 viewMatrix = cameraComponent->getViewMatrix(cameraTransform);
    glm::mat4 projectionMatrix = cameraComponent->getProjectionMatrix();

    // light stuff
    auto lightEntities = scene->getEntities()->getEntitiesWith<LightComponent>();
    // default light values
    glm::vec3 lightDirection = glm::vec3(0.0f, 10.0f, 0.0f);
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    float lightIntensity = 1.0f;

    if (!lightEntities.empty()) {
        auto lightComp = scene->getEntities()->getEntityComponent<LightComponent>(lightEntities[0]);
        lightDirection = lightComp->direction;
        lightColor = lightComp->color;
        lightIntensity = lightComp->intensity;
    }
    // if (!lightEntities.empty()) {
    //     auto lightEntity = lightEntities[0];
    //     auto lightComp = scene->getEntities()->getEntityComponent<LightComponent>(lightEntity);
    //     auto lightTransform = scene->getEntities()->getEntityComponent<TransformComponent>(lightEntity);

    //     if (lightTransform) {
    //         // Assuming forward is -Z in your engine
    //         glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
    //         glm::vec3 direction = glm::mat3(lightTransform->getTransform()) * forward;
    //         lightDirection = glm::normalize(direction);
    //     } else {
    //         lightDirection = lightComp->direction;
    //     }
    //     lightColor = lightComp->color;
    //     lightIntensity = lightComp->intensity;
    // }

    // batch rendering with submeshes
    std::map<MeshMaterialKey, std::vector<glm::mat4>> simpleBatches;
    std::map<MeshSubmeshMaterialKey, std::vector<glm::mat4>> submeshBatches;

    auto list = scene->getEntities()->getEntitiesWith<TransformComponent, MeshComponent, MaterialComponent>();
    for (auto entity : list) {
        auto transformComp = scene->getEntities()->getEntityComponent<TransformComponent>(entity);
        auto meshComp = scene->getEntities()->getEntityComponent<MeshComponent>(entity);
        auto materialComp = scene->getEntities()->getEntityComponent<MaterialComponent>(entity);

        if (!transformComp || !meshComp || !meshComp->mesh || !materialComp || !materialComp->material) {
            continue;
        }

        if (transformComp->dirty) {
            transformComp->updateMatrix();
            transformComp->dirty = false;
        }

        auto mesh = meshComp->mesh;
        auto defaultMaterial = materialComp->material;
        const auto& submeshes = mesh->getSubmeshes();

        if (submeshes.empty()) {
            // simple mesh, no submeshes, rendered simply
            MeshMaterialKey key{mesh, defaultMaterial};
            simpleBatches[key].push_back(transformComp->getTransform());
        } else {
            // has submeshes, render each with its own material
            for (size_t i = 0; i < submeshes.size(); i++) {
                const auto& submesh = submeshes[i];
                auto material = defaultMaterial;

                // check for material, if non stay with default
                if (!submesh.materialName.empty()) {
                    auto it = materialComp->materialsByName.find(submesh.materialName);
                    if (it != materialComp->materialsByName.end()) {
                        material = it->second;
                    }
                }

                MeshSubmeshMaterialKey key{mesh, i, material};
                submeshBatches[key].push_back(transformComp->getTransform());
            }
        }
    }

    // no-submesh rendering
    for (const auto& [key, transforms] : simpleBatches) {
        auto mesh = key.mesh;
        auto material = key.material;

        material->bind();
        auto shader = material->getShader();
        if (!shader) {
            continue;
        }

        shader->setUniformMat4("uView", viewMatrix);
        shader->setUniformMat4("uProjection", projectionMatrix);

        // TODO still have to take care of this one
        uploadInstanceTransforms(mesh, transforms);

        mesh->getVertexArray()->bind();
        if (mesh->useIndices()) {
            glDrawElementsInstanced(GL_TRIANGLES,
                                    static_cast<GLsizei>(mesh->getIndexBuffer()->getCount()),
                                    GL_UNSIGNED_INT,
                                    nullptr,
                                    static_cast<GLsizei>(transforms.size()));
        } else {
            glDrawArraysInstanced(GL_TRIANGLES,
                                  0,
                                  static_cast<GLsizei>(mesh->getVertexCount()),
                                  static_cast<GLsizei>(transforms.size()));
        }
        mesh->getVertexArray()->unbind();
    }

    // submesh rendering
    for (const auto& [key, transforms] : submeshBatches) {
        auto mesh = key.mesh;
        auto submeshIndex = key.submeshIndex;
        auto material = key.material;
        const auto& submesh = mesh->getSubmeshes()[submeshIndex];

        material->bind();
        auto shader = material->getShader();
        if (!shader) {
            continue;
        }

        shader->setUniformMat4("uView", viewMatrix);
        shader->setUniformMat4("uProjection", projectionMatrix);
        shader->setUniformVec3("uLightDirection", lightDirection);
        shader->setUniformVec3("uLightColor", lightColor * lightIntensity);

        uploadInstanceTransforms(mesh, transforms);

        mesh->getVertexArray()->bind();
        if (mesh->useIndices()) {
            glDrawElementsInstanced(GL_TRIANGLES,
                                    static_cast<GLsizei>(submesh.indexCount),
                                    GL_UNSIGNED_INT,
                                    reinterpret_cast<void*>(submesh.indexOffset * sizeof(uint32_t)),
                                    static_cast<GLsizei>(transforms.size()));
        } else {
            // submeshes without indices? even a thing?
            // size_t vertexCount = mesh->getVertexCount();
            // size_t verticesPerSubmesh = vertexCount / mesh->getSubmeshes().size();
            // size_t vertexOffset = submeshIndex * verticesPerSubmesh;

            // glDrawArraysInstanced(GL_TRIANGLES,
            //                       static_cast<GLsizei>(vertexOffset),
            //                       static_cast<GLsizei>(verticesPerSubmesh),
            //                       static_cast<GLsizei>(transforms.size()));
        }
        mesh->getVertexArray()->unbind();
    }

    // batch rendering, get all objects with the same mesh and material, just the transform changes
    // std::map<MeshMaterialKey, std::vector<glm::mat4>> batches;
    // auto list = scene->getEntities()->getEntitiesWith<TransformComponent, MeshComponent, MaterialComponent>();
    // for (auto entity : list) {
    //     auto transformComp = scene->getEntities()->getEntityComponent<TransformComponent>(entity);
    //     auto meshComp = scene->getEntities()->getEntityComponent<MeshComponent>(entity);
    //     auto materialComp = scene->getEntities()->getEntityComponent<MaterialComponent>(entity);

    //     // for each object we add the transform to the right batch
    //     if (transformComp && meshComp && meshComp->mesh && materialComp && materialComp->material) {
    //         MeshMaterialKey key{meshComp->mesh, materialComp->material};
    //         batches[key].push_back(transformComp->getTransform());
    //     }
    // }

    // // render all batches
    // for (const auto& [key, transforms] : batches) {
    //     auto mesh = key.mesh;
    //     auto material = key.material;
    //     material->bind();

    //     auto shader = material->getShader();
    //     if (!shader) {
    //         continue;
    //     }

    //     shader->setUniformMat4("uView", viewMatrix);
    //     shader->setUniformMat4("uProjection", projectionMatrix);

    //     uploadInstanceTransforms(mesh, transforms);

    //     mesh->getVertexArray()->bind();
    //     if (mesh->useIndices()) {
    //         glDrawElementsInstanced(GL_TRIANGLES,
    //                                 static_cast<GLsizei>(mesh->getIndexBuffer()->getCount()),
    //                                 GL_UNSIGNED_INT,
    //                                 nullptr,
    //                                 static_cast<GLsizei>(transforms.size()));
    //     } else {
    //         glDrawArraysInstanced(GL_TRIANGLES,
    //                               0,
    //                               static_cast<GLsizei>(mesh->getVertexCount()),
    //                               static_cast<GLsizei>(transforms.size()));
    //     }
    //     mesh->getVertexArray()->unbind();
    // }

    // render viable entities (old stuff)
    // auto list = scene->getEntities()->getEntitiesWith<TransformComponent, MeshComponent, MaterialComponent>();
    // for (auto entity : list) {
    //     auto transformComp = scene->getEntities()->getEntityComponent<TransformComponent>(entity);
    //     auto meshComp = scene->getEntities()->getEntityComponent<MeshComponent>(entity);
    //     auto materialComp = scene->getEntities()->getEntityComponent<MaterialComponent>(entity);

    //     if (transformComp && meshComp && meshComp->mesh && materialComp && materialComp->material) {
    //         materialComp->material->bind();

    //         auto shader = materialComp->material->getShader();
    //         if (!shader) {
    //             spdlog::warn("Material has no shader, skipping entity {}", entity);
    //             continue;
    //         }

    //         // TODO: should this be in the material as references?
    //         shader->setUniformMat4("uView", viewMatrix);
    //         shader->setUniformMat4("uProjection", projectionMatrix);
    //         shader->setUniformMat4("uTransform", transformComp->transform);

    //         meshComp->mesh->draw();
    //     } else {
    //         spdlog::warn("Entity {} is missing required components for rendering", entity);
    //     }
    // }

    // TODO: handle the skybox some other way, in shader?
    if (m_skyboxEnabled) {
        skybox->render(viewMatrix, projectionMatrix);
    }

    // render each component with a text object
    auto textList = scene->getEntities()->getEntitiesWith<TextComponent>();
    for (auto entity : textList) {
        auto textComp = scene->getEntities()->getEntityComponent<TextComponent>(entity);
        if (textComp) {
            auto font = fonts->get(textComp->fontId);
            if (font) {
                font.value()->draw(textComp->text, textComp->x, textComp->y, textComp->scale, textComp->color);
            } else {
                spdlog::warn("Font not found: {}", textComp->fontId);
            }
        } else {
            spdlog::warn("Text component not found for entity {}", entity);
        }
    }

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

    shaders = std::make_unique<Shaders>();
    auto shaderInit = shaders->init();
    if (!shaderInit) {
        return tl::unexpected(shaderInit.error());
    }

    materials = std::make_unique<Materials>();
    auto materialsInit = materials->init();
    if (!materialsInit) {
        return tl::unexpected(materialsInit.error());
    }

    fonts = std::make_unique<Fonts>();
    if (!fonts->init(shaders->get("default.text").value())) {
        return tl::unexpected(Error{"Failed to initialize fonts"});
    }

    // should also be somewhere else
    setVSync(false);
    setMSAA(true);

    return {};
}

[[nodiscard]] auto Renderer::loadSkybox(const std::vector<std::shared_ptr<Texture>>& faceFiles) -> bool {
    shaders->add(std::make_shared<Shader>("skybox", "resources/shaders/skybox.vert", "resources/shaders/skybox.frag"));
    skybox = std::make_unique<Skybox>();
    skybox->setShader(shaders->get("skybox").value());

    if (skybox->loadFromTextures(faceFiles)) {
        m_skyboxEnabled = true;
        return true;
    }

    m_skyboxEnabled = false;
    return false;
}

auto Renderer::unloadSkybox() -> void {
    if (skybox) {
        skybox->unload();
        m_skyboxEnabled = false;
    } else {
        spdlog::warn("Skybox not loaded, cannot unload");
    }
}

auto Renderer::setVSync(bool enabled) -> void {
    if (enabled) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
    }
}

auto Renderer::setMSAA(bool enabled) -> void {
    if (enabled) {
        glEnable(GL_MULTISAMPLE);
    } else {
        glDisable(GL_MULTISAMPLE);
    }
}

}  // namespace Vengine