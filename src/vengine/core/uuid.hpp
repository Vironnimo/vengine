#pragma once
#include <cstdint>

namespace Vengine {

class UUID {
   public:
    static uint64_t create() {
        return m_nextId++;
    }

    static void reset() {
        m_nextId = 1;
    }

   private:
    static uint64_t m_nextId;
};

}  // namespace Vengine