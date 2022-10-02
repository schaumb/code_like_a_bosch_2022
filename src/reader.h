
#ifndef HACK_LIKE_A_BOSCH_2022_READER_H
#define HACK_LIKE_A_BOSCH_2022_READER_H

#include <string>
#include <vector>
#include <optional>
#include <sstream>
#include <deque>
#include <array>
#include "imgui.h"


enum class ObjType {
    noDetection = 0U,
    truck = 1U,
    car = 2U,
    motorbike = 3U,
    bicycle = 4U,
    pedestrian = 5U,
    carOrTruck = 6U
};

struct Data {
    double time{};
    struct CamData {
        ImVec2 d;
        ImVec2 v;
        ObjType obj_type{};
    } cam_data[15];
    struct CornerData {
        ImVec2 d;
        float d3{};
        ImVec2 v;
        ImVec2 a;
        float prob{};
    } corner_data[10][4];

    friend std::istream &operator>>(std::istream &in, Data &data);
};

struct Object {
    ImVec2 min;
    ImVec2 max;
    ObjType something;
    ImColor color;
};

struct Reader {
    std::vector<std::string> directories;
    const char* selected = "<choose directory>";
    bool loading{};
    std::optional<std::stringstream> input;
    std::size_t max{}, curr{};
    std::deque<Data> file_data;

    explicit Reader();

    void set_selected(const std::string& elem);
    void read_async();

    inline static const auto cornerSensors = std::array<ImVec2, 4>{{{0.6286, -3.4738}, {-0.6286, -3.4738}, {0.738, 0.7664}, {-0.738, 0.7664}}};
    inline static const auto camSensor = ImVec2{0, -1.7826001f - 3.4738f - 0.7664f};

    [[nodiscard]] std::vector<Object> get_objects_at(float time) const;
};


#endif //HACK_LIKE_A_BOSCH_2022_READER_H
