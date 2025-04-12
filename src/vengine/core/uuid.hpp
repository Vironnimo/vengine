#pragma once
#include <cstdint>

namespace Vengine {

class UUID {
   public:
    static auto create() -> uint64_t {
        return m_nextId++;
    }

    static void reset() {
        m_nextId = 1;
    }

   private:
    static uint64_t m_nextId;
};

}  // namespace Vengine