#include "action.hpp"
#include <cassert>

namespace Vengine {

auto Action::execute() -> void {
    if (m_callback) {
        m_callback();
    }
}

auto Action::addKeyBinding(KeyBinding keyBind) -> void {
    assert(keyBind.key != 0 && "KeyBinding key cannot be 0");
    
    m_keyBindings.push_back(keyBind);
}

auto Action::getKeyBindings() -> const std::vector<KeyBinding>& {
    return m_keyBindings;
}

auto Action::getId() const -> const std::string& {
    return m_id;
}

}  // namespace Vengine