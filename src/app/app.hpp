#pragma once

#include <memory>

#include "app/test_module.hpp"
#include "vengine/vengine.hpp"

class App {
   public:
    App();
    void run();

   private:
    std::shared_ptr<Vengine::Vengine> m_vengine;
    std::shared_ptr<TestModule> m_testModule;
};