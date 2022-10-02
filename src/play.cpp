
#include "play.h"
#include "reader.h"

Play::Play(const Reader& r)
    : timeRange{static_cast<float>(r.file_data.front().time),
                static_cast<float>(r.file_data.back().time)}
    , time{timeRange.x}
    , speed{1.0}
    , play{true}
    , end{false}
{

}

void Play::step(float deltaTime) {
    if (end || !play)
        return;

    auto next = time + deltaTime * speed;
    if (next >= timeRange.y) {
        time = timeRange.y;
        end = true;
        play = false;
    } else {
        time = next;
    }
}