#include "action.hpp"

namespace Vengine {

auto Action::execute() -> void {
    if (m_callback) {
        m_callback();
    }
}

auto Action::addKeyBinding(KeyBinding keyBind) -> void {
    m_keyBindings.push_back(keyBind);
}

auto Action::getKeyBindings() -> const std::vector<KeyBinding>& {
    return m_keyBindings;
}

auto Action::getId() const -> const std::string& {
    return m_id;
}
auto Action::getName() const -> const std::string& {
    return m_name;
}

}  // namespace Vengine