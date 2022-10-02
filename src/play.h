
#ifndef HACK_LIKE_A_BOSCH_2022_PLAY_H
#define HACK_LIKE_A_BOSCH_2022_PLAY_H

#include "imgui.h"
struct Reader;

struct Play {
    ImVec2 timeRange;
    float time;
    float speed;

    bool play;
    bool end;

    [[nodiscard]] explicit Play(const Reader&);
};


#endif //HACK_LIKE_A_BOSCH_2022_PLAY_H
