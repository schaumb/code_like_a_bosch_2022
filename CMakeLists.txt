cmake_minimum_required(VERSION 3.22)
project(hack_like_a_bosch_2022)

set(CMAKE_CXX_STANDARD 17)
set(IMGUI_DIR ../imgui)
set(SRC_DIR src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wformat -Os -DIMGUI_DISABLE_FILE_FUNCTIONS -s USE_SDL=2 -s DISABLE_EXCEPTION_CATCHING=1")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=0 -s ASSERTIONS=1 -s NO_FILESYSTEM=1 --shell-file shell_minimal.html -s USE_SDL=2 -s DISABLE_EXCEPTION_CATCHING=1")
include_directories(SDL2Test /usr/include/SDL2 ../emsdk/upstream/emscripten/cache/sysroot/include ${SRC_DIR})

include_directories(${IMGUI_DIR})
add_executable(hack_like_a_bosch_2022
        ${SRC_DIR}/main.cpp ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp ${SRC_DIR}/gui.cpp ${SRC_DIR}/reader.cpp ${SRC_DIR}/play.cpp)

target_link_libraries(hack_like_a_bosch_2022)