#include <imgui.h>
#include "gui.h"

ImVec2 Context::transform_point(const ImVec2& from) const {
    using namespace ImGui;
    ImGuiIO& io = GetIO();

    return {
        from.x * scale + io.DisplaySize.x / 2,
        from.y * scale + io.DisplaySize.y / 2
    };
}

void Context::init() {
}


void Context::add_things() const {
    using namespace ImGui;

    ImDrawList* p = GetWindowDrawList();

    p->AddRectFilled(transform_point({}), transform_point({100, 50}), ImColor{1.0f, 0.f, 0.f, 1.0f});
}