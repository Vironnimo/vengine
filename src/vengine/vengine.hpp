#pragma once

#include <memory>
#include <tl/expected.hpp>

#include "vengine/core/error.hpp"
#include "vengine/renderer/renderer.hpp"
// #include "vengine/resource_manager/resource_manager.hpp"

namespace Vengine {

class Vengine {
   public:
    bool isRunning = false;

    Vengine();
    [[nodiscard]] auto init() -> tl::expected<void, Error>;
    auto run() -> void;

   private:
    // std::unique_ptr<ResourceManager> m_ResourceManager;
    std::unique_ptr<Renderer> m_Renderer;
    std::shared_ptr<Window> m_window;
};

}  // namespace Vengine