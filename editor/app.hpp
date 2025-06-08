#pragma once

#include <vengine/vengine.hpp>

namespace Veditor {

class App {
   public:
    App();
    ~App();

    void run();
    void statsPanel(const std::shared_ptr<Vengine::Vengine>& vengine);

   private:
    std::shared_ptr<Vengine::Vengine> m_vengine;

    auto entitiesPanel(const std::shared_ptr<Vengine::Vengine>& vengine) -> void;
    auto entityNode(const std::shared_ptr<Vengine::Vengine>& vengine, Vengine::EntityId entityId) -> void;
};

}  // namespace Veditor