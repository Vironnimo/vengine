#pragma once

#include <vengine/vengine.hpp>

namespace Veditor {

class App {
   public:
    App();
    ~App();

    void run();

    void gui(const std::shared_ptr<Vengine::Vengine>& vengine);

   private:
    std::shared_ptr<Vengine::Vengine> m_vengine;
};

}  // namespace Veditor