#include "editor_module.hpp"
#include <imgui.h>

namespace Veditor {

void EditorModule::onAttach(Vengine::Vengine& vengine) {
}

void EditorModule::onUpdate(Vengine::Vengine& vengine, float deltaTime) {
    static int i = 0;
    if (i > 5) {
        // ImGui::Text("Hello, world! %d", i);
    }
    i++;
}

void EditorModule::onDetach(Vengine::Vengine& vengine) {
}

}  // namespace Veditor