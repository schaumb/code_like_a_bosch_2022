
#include "reader.h"
#include <filesystem>
#include <iostream>
#include <emscripten.h>
#include <emscripten/fetch.h>
#include "gui.h"

std::istream& operator>>(std::istream& in, SensorData& data) {
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
    for (auto& setter : {+[](SensorData::CamData& cd, float v) { cd.d.x = v / 128; },
                         +[](SensorData::CamData& cd, float v) { cd.d.y = v / 128; },
                         +[](SensorData::CamData& cd, float v) { cd.obj_type = static_cast<ObjType>(v); },
                         +[](SensorData::CamData& cd, float v) { cd.v.x = v / 256; },
                         +[](SensorData::CamData& cd, float v) { cd.v.y = v / 256; }})
        for (auto& camData : data.cam_data)
            setter(camData, get());

    for (std::size_t count = std::size(data.corner_data[0]); count > 0; --count)
        get(); // ignore timestamp

    for (auto& setter : {+[](SensorData::CornerData& cd, float v) { cd.a.x = v / 2048; },
                         +[](SensorData::CornerData& cd, float v) { cd.a.y = v / 2048; },
                         +[](SensorData::CornerData& cd, float v) { cd.d.x = v / 128; },
                         +[](SensorData::CornerData& cd, float v) { cd.d.y = v / 128; },
                         +[](SensorData::CornerData& cd, float v) { cd.d3 = v / 128; },
                         +[](SensorData::CornerData& cd, float v) { cd.prob = v / 128; },
                         +[](SensorData::CornerData& cd, float v) { cd.v.x = v / 256; },
                         +[](SensorData::CornerData& cd, float v) { cd.v.y = v / 256; }})
        for (auto& cornerDataLine : data.corner_data)
            for (auto& cornerData : cornerDataLine)
                setter(cornerData, get());

    get(); get();
    std::getline(ss, buf);
    return in;
}


std::istream &operator>>(std::istream &in, HostData &data) {
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
    data.a.x = get() / 2048;
    data.a.y = get() / 2048;
    data.psiZ = get() / 16384;
    get(); // abs time
    data.v.x = get() / 256;
    data.v.y = get() / 256;

    return in;
}

Reader::Reader()
    : directories {"PSA_ADAS_W3_FC_2022-09-01_14-49_0054.MF4",
                   "PSA_ADAS_W3_FC_2022-09-01_15-03_0057.MF4",
                   "PSA_ADAS_W3_FC_2022-09-01_15-12_0059.MF4",
                   "PSA_ADAS_W3_FC_2022-09-01_15-17_0060.MF4"}
{
}


template<class InMPtr, class DMPtr>
void Reader::read_async() {
    constexpr std::chrono::duration<float, std::ratio<1>> seconds {1.0f/30.0f};
    auto now = std::chrono::system_clock::now();
    auto& [input, max, curr] = *(this->*InMPtr{}());
    auto& container = this->*DMPtr{}();

    typename std::remove_reference_t<decltype(container)>::value_type data;

    while (input >> data) {
        container.emplace_back(data);
        if (std::chrono::duration<float, std::ratio<1>> dur = std::chrono::system_clock::now() - now; dur > seconds) {
            curr = input.tellg();
            return emscripten_async_call(+[](void* arg) { static_cast<Reader*>(arg)->template read_async<InMPtr, DMPtr>(); }, this, 0);
        }
    }
    curr = max;
    (this->*InMPtr{}()).reset();
    --loading;
}

template<auto constant>
using constant_t = std::integral_constant<decltype(constant), constant>;

void Reader::set_selected(const std::string & elem) {
    if (elem == selected)
        return;

    selected = elem.c_str();


    auto fetch = [&] (const char* name, auto inPtr, auto mptr) {
        ++loading;
        (this->*mptr()).clear();
        emscripten_fetch_attr_t attr;
        emscripten_fetch_attr_init(&attr);
        strcpy(attr.requestMethod, "GET");
        attr.userData = this;
        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
        attr.onsuccess = +[] (emscripten_fetch_t *fetch) {
            auto* ptr = static_cast<Reader*>(fetch->userData);
            auto& ref = (ptr->*decltype(inPtr){}()).emplace(fetch->data, fetch->numBytes).ss;

            std::string tmp;
            std::getline(ref, tmp); // header

            emscripten_async_call(+[](void* arg) { static_cast<Reader*>(arg)->template read_async<decltype(inPtr), decltype(mptr)>(); }, ptr, 0);
        };

        attr.onerror = +[] (emscripten_fetch_t *fetch) {
            printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
            emscripten_fetch_close(fetch);
            auto* ptr = static_cast<Reader*>(fetch->userData);
            --ptr->loading;
        };
        emscripten_fetch(&attr, ("." / std::filesystem::path(elem) / name).c_str());
    };
    fetch("Group_349.csv", constant_t<&Reader::sensorInput>{}, constant_t<&Reader::file_data>{});
    fetch("Group_416.csv", constant_t<&Reader::hostInput>{}, constant_t<&Reader::host_data>{});
}

[[nodiscard]] std::vector<Object> Reader::get_objects_at(float time) const {
    auto it = std::lower_bound(file_data.begin(), file_data.end(), time, [](const SensorData& data, float time) {
        return data.time < time;
    });
    std::vector<Object> res;
    if (it != file_data.end()) {
        std::transform(std::begin(it->cam_data), std::end(it->cam_data),
                       std::back_inserter(res), [&] (const SensorData::CamData& cd) {
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
                           std::back_inserter(res), [&] (const SensorData::CornerData& cd) {
                    if (cd.d.x == 0 && cd.d.y == 0) {
                        ++ix;
                        return Object{};
                    }

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

float Reader::get_speed_at(float time) const {
    auto it = std::lower_bound(host_data.begin(), host_data.end(), time, [](const HostData& data, float time) {
        return data.time < time;
    });

    if (it != host_data.end()) {
        return it->v.x * 3.6f;
    } else if (!host_data.empty()) {
        return host_data.back().v.x * 3.6f;
    }

    return 0;
}
