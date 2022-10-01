#ifndef HACK_LIKE_A_BOSCH_2022_GUI_H
#define HACK_LIKE_A_BOSCH_2022_GUI_H

typedef struct SDL_Window SDL_Window;
typedef union SDL_Event SDL_Event;
typedef void* SDL_GLContext;

struct Context {
    SDL_Window *g_Window;
    SDL_GLContext g_GLContext;
    float scale;

    explicit Context(SDL_Window*);
    void the_main_loop();


    void init();
    void event_handler(const SDL_Event&) {}

    [[nodiscard]] ImVec2 transform_point(const ImVec2& from) const;
    [[nodiscard]] ImVec2 transform_size(const ImVec2& from) const;
    [[nodiscard]] float transform_size(const float& from) const;

    void add_things() const;
};

#endif //HACK_LIKE_A_BOSCH_2022_GUI_H
