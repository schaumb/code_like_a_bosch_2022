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

ImVec2 Context::transform_size(const ImVec2& from) const {
    return {
        from.x * scale,
        from.y * scale
    };
}

float Context::transform_size(const float& from) const {
    return from * scale;
}

void Context::init() {
}


void Context::add_things() const {
    using namespace ImGui;

    ImDrawList* p = GetWindowDrawList();

    p->AddRectFilled(transform_point({}), transform_point({100, 50}), ImColor{1.0f, 0.f, 0.f, 1.0f});


    p->AddCircleFilled(transform_point({-10, -10}), transform_size(9), ImColor{0.f, 0.f, 1.f, 1.f});

    p->AddQuadFilled(transform_point({0, 0}), transform_point({-50, 10}), transform_point({-24, 15}), transform_point({-5, 21}), ImColor{0.f, 1.f, 1.f, 1.f});
}