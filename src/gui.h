#ifndef HACK_LIKE_A_BOSCH_2022_GUI_H
#define HACK_LIKE_A_BOSCH_2022_GUI_H

#include <vector>
#include <string>
#include <optional>
#include <new>
#include "reader.h"
#include "play.h"

typedef struct SDL_Window SDL_Window;
typedef union SDL_Event SDL_Event;
typedef void* SDL_GLContext;

struct Context {
    SDL_Window *g_Window;
    SDL_GLContext g_GLContext;
    float scale;
    std::deque<std::string> logs;
    std::optional<Reader> reader;
    std::optional<Play> player;

    explicit Context(SDL_Window*);
    void the_main_loop();

    void draw_background();

    void init();
    void event_handler(const SDL_Event&) {}

    [[nodiscard]] ImVec2 transform_point(const ImVec2& from) const;
    [[nodiscard]] ImVec2 transform_size(const ImVec2& from) const;
    [[nodiscard]] float transform_size(const float& from) const;

    void add_things();
    void create_log_window();

    void add_log(std::string&& text) {
        logs.emplace_back(text);
    }


    static inline const ImColor cornerColors[4] = { {255, 63, 0}, {255, 127, 30}, {255, 191, 50}, {255, 230, 0}};
    static inline const ImColor camColor = ImColor(255, 0, 0);
};

#endif //HACK_LIKE_A_BOSCH_2022_GUI_H
