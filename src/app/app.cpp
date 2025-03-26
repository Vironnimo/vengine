#include "app.hpp"

#include <memory>

#include "vengine/vengine.hpp"

App::App() {
    m_vengine = std::make_unique<Vengine::Vengine>();
}

void App::run() {
    m_vengine->run();
}

int main() {
    App app;
    app.run();

    return 0;
}
