#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <tl/expected.hpp>

#include "vengine/core/error.hpp"

namespace Vengine {

struct WindowParams {
    std::string title = "My App";
    bool centered = true;
    int width = 640;
    int height = 480;
    int widthMin = 0;
    int heightMin = 0;
    int maxFps = 0;
};

class Window {
   public:
    [[nodiscard]] auto init() -> tl::expected<void, Error>;
    [[nodiscard]] auto create(WindowParams& params) -> tl::expected<GLFWwindow*, Error>;

    // get raw GLFWwindow pointer
    [[nodiscard]] auto get() -> GLFWwindow*;

   private:
    GLFWwindow* m_window = nullptr;
};

}  // namespace Vengine