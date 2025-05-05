#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include "events.hpp"

namespace Vengine {

class EventSystem {
   public:
    template <typename EventType>
    using Handler = std::function<void(const EventType&)>;

    template <typename EventType>
    void subscribe(Handler<EventType> handler) {
        auto wrapper = [handler](const Event& e) { handler(static_cast<const EventType&>(e)); };
        m_handlers[typeid(EventType)].push_back(wrapper);
    }

    void publish(const Event& event) {
        auto it = m_handlers.find(event.getType());
        if (it != m_handlers.end()) {
            for (auto& handler : it->second) {
                handler(event);
            }
        }
    }

   private:
    std::unordered_map<std::type_index, std::vector<std::function<void(const Event&)>>> m_handlers;
};

extern EventSystem g_eventSystem;

}  // namespace Vengine