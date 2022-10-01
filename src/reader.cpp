
#include "reader.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <emscripten.h>

std::istream& operator>>(std::istream& in, Data& data) {
    std::string buf;
    in >> std::skipws;
    if (!std::getline(in, buf, ',') || buf.empty()) {
        in.setstate(std::ios::eofbit);
        return in;
    };
    data.time = std::stof(buf);


    auto get = [&] { std::getline(in, buf, ','); return std::stof(buf); };
    get(); // ignore sensor ts data
    for (auto& setter : {+[](Data::CamData& cd, float v) { cd.d.x = v / 128; },
                         +[](Data::CamData& cd, float v) { cd.d.y = v / 128; },
                         +[](Data::CamData& cd, float v) { cd.obj_type = static_cast<ObjType>(v); },
                         +[](Data::CamData& cd, float v) { cd.v.x = v / 256; },
                         +[](Data::CamData& cd, float v) { cd.v.y = v / 256; }})
        for (auto& camData : data.cam_data)
            setter(camData, get());

    for (std::size_t count = std::size(data.corner_data[0]); count > 0; --count)
        get(); // ignore timestamp

    for (auto& setter : {+[](Data::CornerData& cd, float v) { cd.a.x = v / 2048; },
                         +[](Data::CornerData& cd, float v) { cd.a.y = v / 2048; },
                         +[](Data::CornerData& cd, float v) { cd.d.x = v / 128; },
                         +[](Data::CornerData& cd, float v) { cd.d.y = v / 128; },
                         +[](Data::CornerData& cd, float v) { cd.d3 = v / 128; },
                         +[](Data::CornerData& cd, float v) { cd.prob = v / 128; },
                         +[](Data::CornerData& cd, float v) { cd.v.x = v / 256; },
                         +[](Data::CornerData& cd, float v) { cd.v.y = v / 256; }})
        for (auto& cornerDataLine : data.corner_data)
            for (auto& cornerData : cornerDataLine)
                setter(cornerData, get());

    get(); get();
    std::getline(in, buf);
    return in;
}

Reader::Reader() noexcept {
    struct {
        decltype(directories)& dirs;
        auto& operator*() { return *this; }
        auto& operator++() { return *this; }
        auto& operator=(const std::filesystem::directory_entry& e) {
            dirs.emplace_back(e.path().filename());
            return *this;
        }
    } output_iterator{directories};

    std::copy_if(std::filesystem::directory_iterator("."), {}, output_iterator, [](auto&& e){
        return std::filesystem::is_regular_file(e.path() / "Group_349.csv");
    });


}

void Reader::read_async() {
    constexpr std::chrono::duration<float, std::ratio<1>> seconds {1.0f/30.0f};
    auto now = std::chrono::system_clock::now();
    Data data;

    while (*input >> data) {
        file_data.emplace_back(data);
        if (std::chrono::duration<float, std::ratio<1>> dur = std::chrono::system_clock::now() - now; dur > seconds) {
            curr = input->tellg();
            return emscripten_async_call(+[](void* arg) { static_cast<Reader*>(arg)->read_async(); }, this, 0);
        }
    }
    curr = max;
    input.reset();
    loading = false;
}

void Reader::set_selected(const std::string & elem) {
    if (elem == selected)
        return;

    selected = elem.c_str();
    loading = true;

    input.emplace(std::filesystem::path(elem) / "Group_349.csv", std::ios::binary | std::ios::ate);
    max = input->tellg();
    curr = 0;
    input->seekg(0, std::ios::seekdir::beg);

    std::string tmp;
    std::getline(*input, tmp); // read header
    file_data.clear();

    emscripten_async_call(+[](void* arg) { static_cast<Reader*>(arg)->read_async(); }, this, 0);
}
