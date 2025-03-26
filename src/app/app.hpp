#pragma once

#include <memory>

#include "vengine/vengine.hpp"

class App {
   public:
    App();
    void run();

   private:
    std::unique_ptr<Vengine::Vengine> m_vengine;
};