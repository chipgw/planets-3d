#include "planetswindow.h"

/* Here we have a nice lovely unique main loop for using in the browser... */
#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <chrono>

PlanetsWindow* window = nullptr;

typedef std::chrono::high_resolution_clock frame_clock;
using std::chrono::duration_cast;
using std::chrono::microseconds;

frame_clock::time_point last_time;

void main_loop() {
    /* Figure out how long the last frame took to render & display, in microseconds. */
    frame_clock::time_point current = frame_clock::now();
    int delay = duration_cast<microseconds>(current - last_time).count();
    last_time = current;

    window->doFrame(delay);
}

int main(int argc, char* argv[]) {
    window = new PlanetsWindow(argc, argv);

    last_time = frame_clock::now();

    emscripten_set_main_loop(main_loop, 0, 1);

    /* TODO - When, if at all, do we delete the window? */
}
#else

int main(int argc, char* argv[]) {
    PlanetsWindow window(argc, argv);

    return window.run();
}

#endif
