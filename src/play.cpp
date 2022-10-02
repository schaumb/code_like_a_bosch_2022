
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