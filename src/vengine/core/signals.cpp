#include "signals.hpp"

#include <spdlog/spdlog.h>

namespace Vengine {

Signals::Signals() {
    spdlog::debug("Constructor SignalSystem");
}

Signals::~Signals() {
    spdlog::debug("Destructor SignalSystem");
}

void Signals::subscribe(const std::string& eventName, std::function<void(void*)> callback) {
    auto& observers = m_observers[eventName];  // automatically creates field for eventname, if it doesn't exist
    observers.push_back(std::move(callback));
}

void Signals::publish(const std::string& eventName, void* data) {
    auto it = m_observers.find(eventName);
    if (it != m_observers.end()) {
        for (const auto& callback : it->second) {
            callback(data);
        }
    }
}

}  // namespace Vengine