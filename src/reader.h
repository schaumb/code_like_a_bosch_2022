
#ifndef HACK_LIKE_A_BOSCH_2022_READER_H
#define HACK_LIKE_A_BOSCH_2022_READER_H

#include <string>
#include <vector>
#include <optional>
#include <sstream>
#include <deque>
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

    [[nodiscard]] std::vector<std::pair<ImVec2, ImColor>> get_points_at(float time) const;
};


#endif //HACK_LIKE_A_BOSCH_2022_READER_H
