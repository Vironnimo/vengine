#pragma once

#include <functional>
#include <string>
#include <unordered_map>

namespace Vengine {

class Signals {
   public:
    Signals();
    ~Signals();

    void subscribe(const std::string& eventName, std::function<void(void*)> callback);
    void publish(const std::string& eventName, void* data = nullptr);

   private:
    // is vector a bad choise here, since we keep adding/removing callbacks?
    std::unordered_map<std::string, std::vector<std::function<void(void*)>>> m_observers;
};

}  // namespace Vengine