#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <tl/expected.hpp>

#include "vengine/core/error.hpp"

namespace Vengine {

struct WindowParams {
    std::string title = "Vengine";
    bool centered = true;
    int width = 1024;
    int height = 768;
    int widthMin = 0;
    int heightMin = 0;
    int maxFps = 0;
};

class Window {
   public:
    Window();
    ~Window();
    [[nodiscard]] auto init() -> tl::expected<void, Error>;
    [[nodiscard]] auto create(WindowParams& params) -> tl::expected<GLFWwindow*, Error>;

    // get raw GLFWwindow pointer
    [[nodiscard]] auto get() -> GLFWwindow*;

   private:
    GLFWwindow* m_window = nullptr;
};

}  // namespace Vengine