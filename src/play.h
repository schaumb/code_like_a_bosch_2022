
#ifndef HACK_LIKE_A_BOSCH_2022_PLAY_H
#define HACK_LIKE_A_BOSCH_2022_PLAY_H

struct Reader;

struct Play {
    float time{};
    bool end;

    [[nodiscard]] explicit Play(const Reader&);
};


#endif //HACK_LIKE_A_BOSCH_2022_PLAY_H
