#include "app.hpp"

#include <memory>

#include "vengine/core/resource_manager.hpp"
#include "vengine/vengine.hpp"

App::App() {
    m_vengine = std::make_unique<Vengine::Vengine>();

    // m_vengine->resourceManager->load<Vengine::Texture>("texture", "test.jpg");
}

void App::run() {
    m_vengine->run();
}

int main() {
    App app;
    app.run();

    return 0;
}
