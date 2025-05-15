#pragma once

#include <cassert>
#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <spdlog/spdlog.h>
#include "events.hpp"
#include "vengine/core/uuid.hpp"

namespace Vengine {

class EventManager {
   public:
    EventManager() {
        spdlog::debug("Constructor EventSystem");
    }

    ~EventManager() {
        spdlog::debug("Destructor EventSystem");
    }

    template <typename EventType>
    auto subscribe(std::function<void(const EventType&)> callback) -> uint64_t {
        assert(callback != nullptr && "Callback cannot be null");

        auto wrapper = [callback](const Event& e) { callback(static_cast<const EventType&>(e)); };
        auto subscriptionId = UUID::create();

        m_callbacks[typeid(EventType)][subscriptionId] = wrapper;
        return subscriptionId;
    }

    auto publish(const Event& event) -> void {
        auto it = m_callbacks.find(event.getType());

        if (it != m_callbacks.end()) {
            for (const auto& [id, callback] : it->second) {
                callback(event);
            }
        }
    }

    auto unsubscribe(uint64_t subscriptionId) -> void {
        for (auto& [type, callbacks] : m_callbacks) {
            auto it = callbacks.find(subscriptionId);
            if (it != callbacks.end()) {
                callbacks.erase(it);
                return;
            }
        }
    }

    auto clear() -> void {
        m_callbacks.clear();
    }

   private:
    std::unordered_map<std::type_index, std::unordered_map<uint64_t, std::function<void(const Event&)>>> m_callbacks;
};

extern EventManager g_eventManager;

}  // namespace Vengine