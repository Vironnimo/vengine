#pragma once

#include <memory>
#include "entities.hpp"

namespace Vengine {

class BaseSystem {
   public:
    virtual ~BaseSystem() = default;
    virtual void update(std::shared_ptr<Entities> entities, float deltaTime) = 0;

    void setEnabled(bool enabled) {
        m_enabled = enabled;
    }

    [[nodiscard]] auto isEnabled() const -> bool {
        return m_enabled;
    }

   private:
    bool m_enabled = true;
};

}  // namespace Vengine