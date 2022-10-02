#include <imgui.h>
#include "gui.h"
#include <utility>
#include <cmath>
#include <algorithm>
#include <ctime>

constexpr std::string_view fakeLogs[4] = {
    "Accident happened",
    "Nothing creative",
    "Almost crashed",
    "Pedestrian in danger"
};

std::string currentDateTime() {
    time_t     now = time(nullptr);
    struct tm  tstruct{};
    char       buf[80]{};
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

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

void Context::draw_background() {
    using namespace ImGui;
    
    ImGuiIO& io = GetIO();


    ImDrawList* p = GetWindowDrawList();

    // Left line
    p->AddLine({5, 0}, { 5, io.DisplaySize.y }, ImColor{1.f,1.f,1.f,1.f}, 5);
    // Right line
    p->AddLine({io.DisplaySize.x-5, 0}, { io.DisplaySize.x-5, io.DisplaySize.y }, ImColor{1.f,1.f,1.f,1.f}, 5);
    // Middle dotted line
    for (int i = 0; i < 10; ++i) {
        auto y = static_cast<float>(-45 + i * 25);
        p->AddRectFilled(transform_point({-0.5f, y}), transform_point({0.5f, y - 10}), ImColor{ 1.f, 1.f, 1.f, 1.f });
    }
}

void Context::init() {
    reader.emplace();
    srand(time(nullptr));
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

    draw_background();


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

    if (reader->loading && reader->max > 0) {
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
    } else if (player && !player->end) {
        player->step(io.DeltaTime);
    }
    if (player && !player->end) {
        for (auto& o : reader->get_objects_at(player->time)) {
            p->AddCircleFilled(transform_point({(o.min.x + o.max.x)/2.f, (o.min.y + o.max.y) / 2.f}),
                               transform_size(0.4), o.color);
        }
    }
    static_assert(ImGuiKey_L == 557);
    if (ImGui::IsKeyReleased(ImGuiKey_L)) {
        std::string_view textToLog = fakeLogs[rand() % 4];
        std::string datetime = currentDateTime();
        add_log(datetime + " - " + textToLog.data());
    }

    // add car
    p->AddRectFilled(transform_point({-0.738, -3.4738}), transform_point({0.738, 0.7664}), ImColor{ .5f, .5f, .5f, 1.f });
    // add car tyres
    for (auto&& point : {ImVec2{-0.738, 0}, {0.738, 0}, {-0.738, -3.4738 + 0.7664}, {0.738, -3.4738 + 0.7664}}) {
        p->AddRectFilled(transform_point({point.x - 0.1f, point.y - 0.22f}), transform_point({point.x + 0.1f, point.y + 0.22f}), ImColor{.2f, .2f, .2f});
    }

    // add zero coord
    p->AddCircle(transform_point({}), transform_size(0.1), ImColor{ .2f, .2f, .2f, 1.f });


    // add radars
    std::size_t ix{};
    for (auto&& point : Reader::cornerSensors) {
        p->AddCircleFilled(transform_point(point), transform_size(0.3), cornerColors[ix++]);
    }
    // add front camera
    p->AddCircleFilled(transform_point({0, -1.7826001}), transform_size(0.3), ImColor{0.f, 1.f, 1.f, 1.f});



   // Left line
   p->AddLine({5, 0}, { 5, io.DisplaySize.y }, ImColor{1.f,1.f,1.f,1.f}, 5);
   // Right line
   p->AddLine({io.DisplaySize.x-5, 0}, { io.DisplaySize.x-5, io.DisplaySize.y }, ImColor{1.f,1.f,1.f,1.f}, 5);
}