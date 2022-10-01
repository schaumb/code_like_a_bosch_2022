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
    reader.emplace();
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
    scale = std::fmin(io.DisplaySize.x / 20, io.DisplaySize.y / 50);


    ImDrawList* p = GetWindowDrawList();



    ImGui::SetNextWindowPos({0, io.DisplaySize.y - 30});
    ImGui::SetNextWindowSize({300, 30});



    ImGui::BeginDisabled(reader->loading);

    if (ImGui::Begin("File chooser", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration)) {
        ImGui::SetNextItemWidth(300);
        if (ImGui::BeginCombo("##choose", reader->selected,
                              ImGuiComboFlags_HeightSmall | ImGuiComboFlags_NoArrowButton)) {
            for (auto &selectable: reader->directories)
                if (ImGui::Selectable(selectable.c_str())) {
                    reader->set_selected(selectable);
                    player.reset();
                }

            ImGui::EndCombo();
        }
        ImGui::End();
    }

    if (reader->loading) {
        p->AddRectFilled({300, io.DisplaySize.y - 20},
                         {300 +
                          (io.DisplaySize.x - 300) *
                          static_cast<float>(static_cast<double>(reader->curr) / static_cast<double>(reader->max)),
                          io.DisplaySize.y},
                         ImColor{1.f, 0.f, 0.f, 1.f});
    }

    ImGui::EndDisabled();

    if (!reader->loading && !reader->file_data.empty() && !player) {
        player.emplace(*reader);
        player->time = static_cast<float>(reader->file_data.front().time);
    } else if (player && !player->end) {
        player->time += io.DeltaTime;
        if (player->time > reader->file_data.back().time) {
            player->end = true;
        }
    }
    if (player && !player->end) {
        auto&& list = reader->get_points_at(player->time);

        for (auto& [point, color] : list) {
            p->AddCircleFilled(transform_point({point.y, -point.x}), transform_size(0.4), color);
        }
    }


    // add car
    p->AddRectFilled(transform_point({-0.6286, -3.4738}), transform_point({0.738, 0.7664}), ImColor{ .5f, .5f, .5f, 1.f });

    // add radars
    for (auto&& point : {ImVec2{0.6286, -3.4738}, {0.738, 0.7664}, {-0.6286, -3.4738}, {-0.738, 0.7664}}) {
        p->AddCircleFilled(transform_point(point), transform_size(0.5), ImColor{0.f, 1.f, 0.f, 1.f});
    }
    // add front camera
    p->AddCircleFilled(transform_point({0, -1.7826001}), transform_size(0.5), ImColor{0.f, 1.f, 1.f, 1.f});

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

   // p->AddCircle(transform_point({}), 50, ImColor{ 1.f, 0.f, 0.f, 1.f});
}