#include <planet.h>
#include <planetsuniverse.h>
#include <chrono>

using namespace std::chrono;

int main() {
    PlanetsUniverse universe;

    /* Use a constant seed for consistency. */
    universe.randSeed(0);

    /* Test a thousand steps. */
    universe.stepsPerFrame = 1000;

    uint32_t sizes[] = { 20, 100, 500 };

    for (uint32_t size : sizes) {
        universe.generateRandom(size, 1000.0f, 1.0f, 1000.0f);

        printf("Simulating %i planets...\n", size);

        high_resolution_clock::time_point start = high_resolution_clock::now();

        universe.advance(10.0f);

        high_resolution_clock::time_point end = high_resolution_clock::now();

        auto delay = duration_cast<milliseconds>(end - start).count();

        printf("%i steps with %i planets took %ims, average step time: %fms.\n", universe.stepsPerFrame, size, delay, delay / float(universe.stepsPerFrame));
        printf("Ended with %i planets.\n", universe.size());

        fflush(stdout);

        /* Clear the thing for the next one. */
        universe.deleteAll();
    }

    return 0;
}
