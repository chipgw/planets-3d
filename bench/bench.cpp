#include <planet.h>
#include <planetsuniverse.h>
#include <chrono>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace std::chrono;

#ifdef EMSCRIPTEN
int bench() {
#else
int main() {
#endif
    PlanetsUniverse universe;

    /* Use a constant seed for consistency. */
    universe.randSeed(0);

    /* Start with a thousand steps. */
    universe.stepsPerFrame = 2000;

    size_t sizes[] = { 20, 100, 200, 500, 800 };

    /* Col:  |- 6-||-- 8--||---   16   ---||---   16   ---| doesn't matter,  Align left. */
    cout << "steps planets total time      average step    remaining planets" << left << endl;

    for (size_t size : sizes) {
        universe.generateRandom(size, 1000.0f, 1.0f, 1000.0f);

        cout << setw(6) << universe.stepsPerFrame
             << setw(8) << size;

        high_resolution_clock::time_point start = high_resolution_clock::now();

        universe.advance(10.0f);

        high_resolution_clock::time_point end = high_resolution_clock::now();

        double delay = duration_cast<duration<double, std::milli>>(end - start).count();

        cout << setw(16) << to_string(delay) + "ms"
             << setw(16) << to_string(delay / double(universe.stepsPerFrame)) + "ms"
             << universe.size() << endl;

        fflush(stdout);

        /* Clear the thing for the next one. */
        universe.deleteAll();

        /* Reduce the number of steps as the amount of planets increases. */
        universe.stepsPerFrame -= 250;
    }

    return 0;
}
