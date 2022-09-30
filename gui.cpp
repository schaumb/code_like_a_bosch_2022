#include <imgui.h>

#include "gui.h"

void Context::init() {

}

void Context::add_things() {
    using namespace ImGui;

    [[maybe_unused]] ImGuiIO& io = GetIO();

    ImDrawList* p = GetWindowDrawList();

    p->AddRectFilled({}, {100, 50}, ImColor{1.0f, 0.f, 0.f, 1.0f});
}