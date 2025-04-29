#pragma once

#include <memory>
#include <string>
#include <spdlog/spdlog.h>

#include "vengine/ecs/entities.hpp"

namespace Vengine {

class Vengine;  

class Scene {
   public:
    Scene(std::string name) : m_name(std::move(name)) {
    }
    virtual ~Scene() = default;

    virtual void load(Vengine& vengine) = 0;
    virtual void cleanup(Vengine& vengine) = 0;

    auto setEntities(std::shared_ptr<Entities> entities) -> void {
        m_entities = std::move(entities);
    }

    [[nodiscard]] auto getEntities() const -> std::shared_ptr<Entities> {
        return m_entities;
    }

    [[nodiscard]] auto getName() const -> const std::string& {
        return m_name;
    }

    auto clearEntities() -> void {
        if (m_entities) {
            m_entities->clear();
        }
    }

   protected:
    std::string m_name;
    
   private:
    std::shared_ptr<Entities> m_entities;
};

}  // namespace Vengine