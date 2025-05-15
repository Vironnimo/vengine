#pragma once

#include <functional>
#include <vector>
#include <string>

namespace Vengine {

struct KeyBinding {
    int key;  // GLFW key id
    bool shift = false;
    bool ctrl = false;
    bool alt = false;
};

class Action {
   public:
    Action(std::string id, std::function<void()> callback)
        : m_id(std::move(id)), m_callback(std::move(callback)) {
    }

    auto execute() -> void;
    auto addKeyBinding(KeyBinding keyBind) -> void;
    auto getKeyBindings() -> const std::vector<KeyBinding>&;
    [[nodiscard]] auto getId() const -> const std::string&;
    [[nodiscard]] auto getName() const -> const std::string&;

   private:
    std::string m_id;
    std::function<void()> m_callback;
    std::vector<KeyBinding> m_keyBindings;
};

}  // namespace Vengine