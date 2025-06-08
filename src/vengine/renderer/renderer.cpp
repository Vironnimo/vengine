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
    // Early safety checks
    if (!mesh) {
        spdlog::error("Null mesh in uploadInstanceTransforms");
        return;
    }

    if (!mesh->getVertexArray()) {
        spdlog::error("Mesh has no vertex array. Ensure finalizeOnMainThread was called.");
        return;
    }

    GLuint vaoID = mesh->getVertexArray()->getID();
    if (vaoID == 0) {
        spdlog::error("Invalid vertex array ID (0) in uploadInstanceTransforms");
        return;
    }

    static std::unordered_map<size_t, GLuint> instanceVBOs;
    size_t meshId = reinterpret_cast<size_t>(mesh.get());
    GLuint instanceVBO = 0;

    if (instanceVBOs.find(meshId) == instanceVBOs.end()) {
        glGenBuffers(1, &instanceVBO);
        instanceVBOs[meshId] = instanceVBO;
    } else {
        instanceVBO = instanceVBOs[meshId];
    }

    // Now safe to bind
    glBindVertexArray(vaoID);
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
    m_drawCallCount = 0;
    m_vertexCount = 0;
    m_triangleCount = 0;

    if (m_preRenderCallback) {
        m_preRenderCallback();
    }

    // light stuff
    auto lightEntities = scene->getEntities()->getEntitiesWith<LightComponent>();
    // default light values
    glm::vec3 lightDirection = glm::vec3(-0.5f, -0.7f, -0.5f);
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    float lightIntensity = 1.0f;
    glm::vec3 lightPos = glm::vec3(20.0f, 50.0f, 20.0f);  // default

    if (!lightEntities.empty()) {
        auto lightComp = scene->getEntities()->getEntityComponent<LightComponent>(lightEntities[0]);
        lightDirection = lightComp->direction;
        lightColor = lightComp->color;
        lightIntensity = lightComp->intensity;
        auto lightTransform = scene->getEntities()->getEntityComponent<TransformComponent>(lightEntities[0]);
        lightPos = lightTransform->getPosition();
    }
    // --- SHADOW MAP PASS ---
    // 1. Set viewport to shadow map size
    glCullFace(GL_FRONT);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, 2048, 2048);
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // 2. Use your depth-only shader
    m_shadowShader->bind();

    // 3. Compute light's view-projection matrix (orthographic for directional light)
    glm::vec3 lightTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 lightUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 lightView = glm::lookAt(lightPos, lightTarget, lightUp);
    float orthoSize = 100.0f;
    glm::mat4 lightProj = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 1.0f, 200.0f);
    glm::mat4 lightSpaceMatrix = lightProj * lightView;
    m_shadowShader->setUniformMat4("uLightSpaceMatrix", lightSpaceMatrix);

    // 4. Batch shadow casters by mesh
    std::map<std::shared_ptr<Mesh>, std::vector<glm::mat4>> shadowBatches;
    auto shadowCasters = scene->getEntities()->getEntitiesWith<TransformComponent, MeshComponent>();
    for (auto entity : shadowCasters) {
        auto transformComp = scene->getEntities()->getEntityComponent<TransformComponent>(entity);
        auto meshComp = scene->getEntities()->getEntityComponent<MeshComponent>(entity);
        if (!transformComp || !meshComp || !meshComp->mesh) {
            continue;
        }

        if (transformComp->dirty) {
            transformComp->updateMatrix();
            transformComp->dirty = false;
        }

        shadowBatches[meshComp->mesh].push_back(transformComp->getTransform());
    }

    // Add ModelComponent entities to shadow casting
    auto modelShadowCasters = scene->getEntities()->getEntitiesWith<TransformComponent, ModelComponent>();
    for (auto entity : modelShadowCasters) {
        auto transformComp = scene->getEntities()->getEntityComponent<TransformComponent>(entity);
        auto modelComp = scene->getEntities()->getEntityComponent<ModelComponent>(entity);

        if (!transformComp || !modelComp || !modelComp->model) {
            continue;
        }

        auto model = modelComp->model;
        auto mesh = model->getMesh();

        if (!mesh || !mesh->getVertexArray()) {
            continue;
        }

        if (transformComp->dirty) {
            transformComp->updateMatrix();
            transformComp->dirty = false;
        }

        shadowBatches[mesh].push_back(transformComp->getTransform());
    }

    // 5. Draw each batch with instancing
    for (const auto& [mesh, transforms] : shadowBatches) {
        uploadInstanceTransforms(mesh, transforms);
        mesh->getVertexArray()->bind();
        if (mesh->useIndices()) {
            glDrawElementsInstanced(GL_TRIANGLES,
                                    static_cast<GLsizei>(mesh->getIndexBuffer()->getCount()),
                                    GL_UNSIGNED_INT,
                                    nullptr,
                                    static_cast<GLsizei>(transforms.size()));

            m_drawCallCount++;
            m_vertexCount += mesh->getIndexBuffer()->getCount() * transforms.size();
            m_triangleCount += mesh->getIndexBuffer()->getCount() / 3 * transforms.size();
        } else {
            glDrawArraysInstanced(GL_TRIANGLES,
                                  0,
                                  static_cast<GLsizei>(mesh->getVertexCount()),
                                  static_cast<GLsizei>(transforms.size()));

            m_drawCallCount++;
            m_vertexCount += mesh->getVertexCount() * transforms.size();
            m_triangleCount += mesh->getVertexCount() / 3 * transforms.size();
        }
        mesh->getVertexArray()->unbind();
    }

    // 6. Unbind framebuffer and reset viewport to window size
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_window->getWidth(), m_window->getHeight());
    glCullFace(GL_BACK);

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

    // TEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEST
    // Render entities with ModelComponent
    auto modelEntities = scene->getEntities()->getEntitiesWith<TransformComponent, ModelComponent>();
    for (auto entity : modelEntities) {
        auto transformComp = scene->getEntities()->getEntityComponent<TransformComponent>(entity);
        auto modelComp = scene->getEntities()->getEntityComponent<ModelComponent>(entity);

        if (!transformComp || !modelComp || !modelComp->model) {
            spdlog::warn("Model entity {} has invalid transform or model", entity);
            continue;
        }

        auto model = modelComp->model;
        auto mesh = model->getMesh();
        auto defaultMaterial = model->getDefaultMaterial();

        if (!mesh || !defaultMaterial) {
            spdlog::warn("Model entity {} has invalid mesh or material", entity);
            continue;
        }

        // Skip if vertex array is still not available
        if (!mesh->getVertexArray()) {
            spdlog::error("Model mesh has no vertex array after initialization attempt");
            continue;
        }

        if (transformComp->dirty) {
            transformComp->updateMatrix();
            transformComp->dirty = false;
        }

        // Rest of the code remains unchanged
        const auto& submeshes = mesh->getSubmeshes();
        if (submeshes.empty()) {
            // Simple mesh, no submeshes
            MeshMaterialKey key{mesh, defaultMaterial};
            simpleBatches[key].push_back(transformComp->getTransform());
        } else {
            // Has submeshes, render each with its material
            for (size_t i = 0; i < submeshes.size(); i++) {
                const auto& submesh = submeshes[i];
                auto material = defaultMaterial;

                // Check if we have a specific material for this submesh
                if (!submesh.materialName.empty()) {
                    material = model->getMaterialForSubmesh(submesh.materialName);
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

        shader->setUniformMat4("uLightSpaceMatrix", lightSpaceMatrix);
        shader->setUniformVec3("uLightDirection", lightDirection);
        shader->setUniformVec3("uLightColor", lightColor * lightIntensity);
        shader->setUniformFloat("uAmbientStrength", 0.4f);
        shader->setUniformVec3("uViewPos", cameraTransform->getPosition());
        // shader->setUniformFloat("uShadowIntensity", 0.5f);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_shadowMap);
        shader->setUniform1i("uShadowMap", 1);

        uploadInstanceTransforms(mesh, transforms);

        mesh->getVertexArray()->bind();
        if (mesh->useIndices()) {
            glDrawElementsInstanced(GL_TRIANGLES,
                                    static_cast<GLsizei>(mesh->getIndexBuffer()->getCount()),
                                    GL_UNSIGNED_INT,
                                    nullptr,
                                    static_cast<GLsizei>(transforms.size()));

            m_drawCallCount++;
            m_vertexCount += mesh->getIndexBuffer()->getCount() * transforms.size();
            m_triangleCount += mesh->getIndexBuffer()->getCount() / 3 * transforms.size();
        } else {
            glDrawArraysInstanced(GL_TRIANGLES,
                                  0,
                                  static_cast<GLsizei>(mesh->getVertexCount()),
                                  static_cast<GLsizei>(transforms.size()));

            m_drawCallCount++;
            m_vertexCount += mesh->getVertexCount() * transforms.size();
            m_triangleCount += mesh->getVertexCount() / 3 * transforms.size();
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

        shader->setUniformMat4("uLightSpaceMatrix", lightSpaceMatrix);
        shader->setUniformMat4("uView", viewMatrix);
        shader->setUniformMat4("uProjection", projectionMatrix);
        shader->setUniformVec3("uLightDirection", lightDirection);
        shader->setUniformVec3("uLightColor", lightColor * lightIntensity);

        shader->setUniformFloat("uAmbientStrength", 0.4f);
        shader->setUniformVec3("uViewPos", cameraTransform->getPosition());

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_shadowMap);
        shader->setUniform1i("uShadowMap", 1);

        uploadInstanceTransforms(mesh, transforms);

        mesh->getVertexArray()->bind();
        if (mesh->useIndices()) {
            glDrawElementsInstanced(GL_TRIANGLES,
                                    static_cast<GLsizei>(submesh.indexCount),
                                    GL_UNSIGNED_INT,
                                    reinterpret_cast<void*>(submesh.indexOffset * sizeof(uint32_t)),
                                    static_cast<GLsizei>(transforms.size()));

            m_drawCallCount++;
            m_vertexCount += submesh.indexCount * transforms.size();
            m_triangleCount += submesh.indexCount / 3 * transforms.size();
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

    // TODO: handle the skybox some other way, in shader?
    if (m_skyboxEnabled) {
        skybox->render(viewMatrix, projectionMatrix);
        m_drawCallCount++;
    }

    // render each component with a text object
    auto textList = scene->getEntities()->getEntitiesWith<TextComponent>();
    for (auto entity : textList) {
        auto textComp = scene->getEntities()->getEntityComponent<TextComponent>(entity);
        if (textComp) {
            auto font = fonts->get(textComp->fontId);
            if (font) {
                m_drawCallCount++;
                font.value()->draw(textComp->text, textComp->x, textComp->y, textComp->scale, textComp->color);
            } else {
                spdlog::warn("Font not found: {}", textComp->fontId);
            }
        } else {
            spdlog::warn("Text component not found for entity {}", entity);
        }
    }

    if (m_postRenderCallback) {
        m_postRenderCallback();
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

    materials = std::make_unique<Materials>();
    auto materialsInit = materials->init();
    if (!materialsInit) {
        return tl::unexpected(materialsInit.error());
    }

    // should also be somewhere else
    setVSync(false);
    setMSAA(true);

    // test shadow stuff
    glGenFramebuffers(1, &m_shadowFBO);
    glGenTextures(1, &m_shadowMap);
    glBindTexture(GL_TEXTURE_2D, m_shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 2048, 2048, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    // Set required texture parameters for shadow mapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        spdlog::error("Shadow framebuffer not complete!");
        return tl::unexpected(Error{"Failed to create shadow framebuffer"});
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return {};
}

[[nodiscard]] auto Renderer::loadSkybox(const std::vector<std::shared_ptr<Texture>>& faceFiles,
                                        std::shared_ptr<Shader> shader) -> bool {
    skybox = std::make_unique<Skybox>();
    skybox->setShader(std::move(shader));

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

auto Renderer::getDrawCallCount() const -> size_t {
    return m_drawCallCount;
}

auto Renderer::getVertexCount() const -> size_t {
    return m_vertexCount;
}

auto Renderer::getTriangleCount() const -> size_t {
    return m_triangleCount;
}

auto Renderer::setVSync(bool enabled) -> void {
    if (enabled) {
        glfwSwapInterval(1);
        m_vsyncEnabled = true;
    } else {
        glfwSwapInterval(0);
        m_vsyncEnabled = false;
    }
}

auto Renderer::setMSAA(bool enabled) -> void {
    if (enabled) {
        glEnable(GL_MULTISAMPLE);
        m_msaaEnabled = true;
    } else {
        glDisable(GL_MULTISAMPLE);
        m_msaaEnabled = false;
    }
}

auto Renderer::initFonts(std::shared_ptr<Shader> fontShader) -> tl::expected<void, Error> {
    fonts = std::make_unique<Fonts>();
    auto fontInit = fonts->init(std::move(fontShader));
    if (!fontInit) {
        return tl::unexpected(fontInit.error());
    }
    return {};
}

auto Renderer::setShadowShader(std::shared_ptr<Shader> shader) -> void {
    m_shadowShader = std::move(shader);
}

auto Renderer::isVSyncEnabled() const -> bool {
    return m_vsyncEnabled;
}

auto Renderer::isMsaaEnabled() const -> bool {
    return m_msaaEnabled;
}

}  // namespace Vengine