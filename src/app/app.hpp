#pragma once

#include <memory>

#include "vengine/vengine.hpp"

class App {
   public:
    App();
    void run();

   private:
    std::shared_ptr<Vengine::Vengine> m_vengine;
};