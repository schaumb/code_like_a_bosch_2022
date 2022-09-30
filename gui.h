#ifndef HACK_LIKE_A_BOSCH_2022_GUI_H
#define HACK_LIKE_A_BOSCH_2022_GUI_H

typedef struct SDL_Window SDL_Window;
typedef union SDL_Event SDL_Event;
typedef void* SDL_GLContext;

struct Context {
    SDL_Window *g_Window;
    SDL_GLContext g_GLContext;

    explicit Context(SDL_Window*);
    void the_main_loop();

    void event_handler(const SDL_Event&) {}

    void add_things();
};

#endif //HACK_LIKE_A_BOSCH_2022_GUI_H
