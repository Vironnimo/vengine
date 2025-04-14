#pragma once

#include <memory>

#include "app/test_layer.hpp"
#include "vengine/vengine.hpp"

class App {
   public:
    App();
    void run();

   private:
    std::shared_ptr<Vengine::Vengine> m_vengine;
    std::shared_ptr<TestLayer> m_testLayer;
};