#include "window.hpp"

#include <glad/glad.h>
#include <spdlog/spdlog.h>


#include "vengine/core/error.hpp"

namespace Vengine {

[[nodiscard]] Window::Window()
{
    spdlog::debug("Constructor Window");
}

Window::~Window()
{
    spdlog::debug("Destructor Window");
    if (m_window != nullptr) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

auto Window::init() -> tl::expected<void, Error> {
    if (glfwInit() == 0) {
        return tl::unexpected(Error{"Failed to initialize GLFW"});
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    return {};
}

[[nodiscard]] auto Window::create(WindowParams& params) -> tl::expected<GLFWwindow*, Error> {
    m_window = glfwCreateWindow(params.width, params.height, params.title.c_str(), nullptr, nullptr);
    if (m_window == nullptr) {
        glfwTerminate();
        return tl::unexpected(Error{"Failed to create GLFW window"});
    }

    glfwMakeContextCurrent(m_window);

    return m_window;
}

auto Window::get() -> GLFWwindow* {
    return m_window;
}

}  // namespace Vengine