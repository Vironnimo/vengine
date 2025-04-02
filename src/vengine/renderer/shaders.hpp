#pragma once

#include <memory>
#include <tl/expected.hpp>

#include "vengine/core/error.hpp"
#include "vengine/renderer/shader.hpp"

namespace Vengine {

///////////////
// For each frame:
    // For each render pass (shadow, main, post-processing, etc.):
    //     For each material type:
    //         1. Bind the appropriate shader program
    //         2. Set global uniforms (camera, lights, time)
    //         3. For each object using this material:
    //             a. Set object-specific uniforms (transform, color)
    //             b. Bind object geometry (VAO)
    //             c. Draw the object


// How Many Shaders in a Game Engine?
// A small game engine might have anywhere from 5-20 shader programs, while larger engines can have dozens or even hundreds. Here's a breakdown of common shader programs you might need:

// Basic Shader Programs
// Standard 3D shader - For rendering normal 3D objects
// Unlit shader - For objects that don't respond to lighting
// Skybox shader - For rendering skyboxes/environment maps
// UI shader - For 2D user interface elements
// Sprite shader - For 2D sprites and billboards
// Text shader - Specialized for rendering text
// More Advanced Shader Programs
// PBR shader - Physically-based rendering
// Particle shader - For particle systems
// Shadow map shader - For generating shadow maps
// Post-processing shaders - Bloom, color correction, blur, etc.
// Terrain shader - Specialized for terrain rendering
// Water shader - For water surfaces and effects
// When to Switch Shader Programs
// You typically switch shader programs when:

// Different material types: Metal vs. wood vs. glass
// Different rendering techniques: Forward vs. deferred rendering
// Different rendering passes: Normal rendering vs. shadow casting
// Different visual effects: Normal objects vs. particle systems


// Let's say you're making a simple 3D game. You might have:

// StandardShader - For most opaque objects
// TerrainShader - For the ground/landscape
// SkyboxShader - For the sky
// UIShader - For menus and HUD
///////////////



///////////////////////////
// Shader Components
// Shader Class (Shader):

// Represents a compiled shader program (vertex + fragment pair)
// Loads shader source from files
// Compiles and links shaders
// Provides binding/unbinding functionality
// Has methods for setting uniforms

// Shaders Manager (Shaders):
// Maintains a collection of shader programs
// Provides access to shaders by name
// Handles shader lifecycle (loading, reloading, unloading)
// Typical Rendering Workflow (???)

// 1. Initialize shaders at startup
// 2. For each frame:
//    a. Clear the screen
//    b. For each object or batch:
//       i.   Get appropriate shader program
//       ii.  Bind the shader program
//       iii. Set global uniforms (camera, lights)
//       iv.  Set object-specific uniforms (transform, color)
//       v.   Bind object geometry (VAO)
//       vi.  Draw the object
//       vii. Unbind (optional)

/////////////////////////


class Shaders {
   public:
    Shaders();
    ~Shaders();
    [[nodiscard]] auto init() -> tl::expected<void, Error>;

    auto add(std::shared_ptr<Shader> shader) -> void;
    [[nodiscard]] auto get(const std::string& name) -> tl::expected<std::shared_ptr<Shader>, Error>;
    [[nodiscard]] auto exists(const std::string& name) const -> bool;
    auto remove(const std::string& name) -> void;
    auto clear() -> void;

   private:
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
};

}  // namespace Vengine