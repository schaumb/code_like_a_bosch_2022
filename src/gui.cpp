#include <imgui.h>
#include "gui.h"
#include <utility>

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

void Context::create_log_window() {
    using namespace ImGui;
    
    ImGuiIO& io = GetIO();

    ImVec2 windowSize = { 300, io.DisplaySize.y };

    ImGui::SetNextWindowPos({ io.DisplaySize.x - windowSize.x, io.DisplaySize.y - windowSize.y });
    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
    

    ImGui::Begin("Log", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysVerticalScrollbar 
    | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    for (auto it = logs.rbegin(); it != logs.rend(); ++it) {
        ImGui::TextWrapped("%s", it->c_str());
    }
    ImGui::End();
}


void Context::add_things() {
    using namespace ImGui;
    ImGuiIO& io = GetIO();
    scale = std::fmin(io.DisplaySize.x / 210, io.DisplaySize.y / 110);


    ImDrawList* p = GetWindowDrawList();

    /*
    p->AddRectFilled(transform_point({}), transform_point({100, 50}), ImColor{1.0f, 0.f, 0.f, 1.0f});
    p->AddCircleFilled(transform_point({-10, -10}), transform_size(9), ImColor{0.f, 0.f, 1.f, 1.f});
    p->AddQuadFilled(transform_point({0, 0}), transform_point({-50, 10}), transform_point({-24, 15}), transform_point({-5, 21}), ImColor{0.f, 1.f, 1.f, 1.f});
    */

   // Left line
   p->AddLine({5, 0}, { 5, io.DisplaySize.y }, ImColor{1.f,1.f,1.f,1.f}, 5);
   // Right line
   p->AddLine({io.DisplaySize.x-5, 0}, { io.DisplaySize.x-5, io.DisplaySize.y }, ImColor{1.f,1.f,1.f,1.f}, 5);
   // Middle dotted line
   p->AddRectFilled(transform_point({0, -50}), transform_point({1, -55}), ImColor{ 1.f, 1.f, 1.f, 1.f });

   p->AddCircle(transform_point({}), 50, ImColor{ 1.f, 0.f, 0.f, 1.f});
}