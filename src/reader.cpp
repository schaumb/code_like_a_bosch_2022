
#include "reader.h"
#include <filesystem>
#include <iostream>
#include <emscripten.h>
#include <emscripten/fetch.h>
#include "gui.h"

std::istream& operator>>(std::istream& in, Data& data) {
    std::string buf;
    if (!std::getline(in >> std::skipws, buf) || buf.empty() || buf.size() < 10) {
        in.setstate(std::ios::eofbit);
        return in;
    }

    std::stringstream ss{buf};

    auto get = [&] {
        std::getline(ss, buf, ',');
        return std::stof(buf);
    };
    data.time = get();
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
    std::getline(ss, buf);
    return in;
}

Reader::Reader()
    : directories {"PSA_ADAS_W3_FC_2022-09-01_14-49_0054.MF4",
                   "PSA_ADAS_W3_FC_2022-09-01_15-03_0057.MF4",
                   "PSA_ADAS_W3_FC_2022-09-01_15-12_0059.MF4",
                   "PSA_ADAS_W3_FC_2022-09-01_15-17_0060.MF4"}
{
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
    file_data.clear();
    curr = max = 0;

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.userData = this;
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = +[] (emscripten_fetch_t *fetch) {
        auto* ptr = static_cast<Reader*>(fetch->userData);
        ptr->max = fetch->numBytes;
        ptr->input.emplace(fetch->data);

        std::string tmp;
        std::getline(*ptr->input, tmp);

        emscripten_async_call(+[](void* arg) { static_cast<Reader*>(arg)->read_async(); }, ptr, 0);
    };

    attr.onerror = +[] (emscripten_fetch_t *fetch) {
        printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
        emscripten_fetch_close(fetch);
        auto* ptr = static_cast<Reader*>(fetch->userData);
        ptr->loading = false;
    };
    emscripten_fetch(&attr, ("." / std::filesystem::path(elem) / "Group_349.csv").c_str());
}

[[nodiscard]] std::vector<Object> Reader::get_objects_at(float time) const {
    auto it = std::lower_bound(file_data.begin(), file_data.end(), time, [](const Data& data, float time) {
        return data.time < time;
    });
    std::vector<Object> res;
    if (it != file_data.end()) {
        std::transform(std::begin(it->cam_data), std::end(it->cam_data),
                       std::back_inserter(res), [&] (const Data::CamData& cd) {
            if (cd.obj_type == ObjType::noDetection)
                return Object {};

            ImVec2 pos{cd.d.y + Reader::camSensor.x, -cd.d.x + Reader::camSensor.y};
            return Object {
                pos, pos, cd.obj_type, Context::camColor
            };
        });
        std::size_t ix{};

        for (auto& d : it->corner_data) {
            std::transform(std::begin(d), std::end(d),
                           std::back_inserter(res), [&] (const Data::CornerData& cd) {
                    if (cd.d.x == 0 && cd.d.y == 0)
                        return Object{};

                    ImVec2 pos{cd.d.y + Reader::cornerSensors[ix].x, -cd.d.x + Reader::cornerSensors[ix].y};
                    return Object {
                        pos, pos, {}, Context::cornerColors[ix++ % 4]
                    };
                });
        }
        res.erase(std::remove_if(res.begin(), res.end(), [](const Object& ic) { return ic.min.x == 0 && ic.min.y == 0; }), res.end());
    }
    return res;
}
