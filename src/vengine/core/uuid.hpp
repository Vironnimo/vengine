#pragma once
#include <cstdint>
#include <list>
#include <string>

namespace Vengine {

class UUID {
   public:
    static auto create() -> uint64_t {
        if (!m_freeIds.empty()) {
            uint64_t id = m_freeIds.front();
            m_freeIds.pop_front();
            return id;
        }

        return m_nextId++;
    }

    static void reset() {
        m_nextId = 1;
        m_freeIds.clear();
    }

    static void free(uint64_t id) {
        m_freeIds.push_back(id);
    }

    static auto toString() -> std::string {
        return "UUID: nextId: " + std::to_string(m_nextId) + ", freeIds: " + std::to_string(m_freeIds.size());
    }

   private:
    static uint64_t m_nextId;
    static std::list<uint64_t> m_freeIds;
};

}  // namespace Vengine