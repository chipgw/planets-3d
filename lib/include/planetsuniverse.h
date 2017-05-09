#pragma once

#include "types.h"
#include <map>
#include <random>
#include <string>
#include <glm/mat4x4.hpp>

#ifdef EMSCRIPTEN
#include "planet.h"
#endif

class PlanetsUniverse {
public:
    typedef std::vector<Planet> list_type;
    typedef list_type::iterator iterator;
    typedef list_type::const_iterator const_iterator;

    std::mt19937 generator;

private:
    list_type planets;

public:
    /* The factor for apparent velocity.
     * (UI velocity * this = actual velocity, because it would be really really small if done directly.) */
    constexpr static float velocityFactor = 1.0e-5f;

    /* UI limits on planet size. */
    constexpr static float minimumMass = 1.0f;
    constexpr static float maximumMass = 1.0e9f;

    std::vector<glm::vec3>::size_type pathLength = 200;
    float pathRecordDistance = 0.25f;

    key_type selected = -1, following = -1;

    /* Speed multiplier for simulation. */
    float simulationSpeed = 1.0f;
    /* How many sub-steps to perform per frame for better accuracy. */
    int stepsPerFrame = 20;

    /* Make new planets. */
    inline key_type addPlanet(const Planet& planet) { planets.push_back(planet); return planets.size() - 1; }
    EXPORT void generateRandom(const size_t& count, const float& positionRange, const float& maxVelocity, const float& maxMass);
    EXPORT key_type addOrbital(Planet& around, const float& radius, const float& mass, const glm::mat4& plane);
    EXPORT void generateRandomOrbital(const size_t& count, key_type target);

#ifndef EMSCRIPTEN
    /* Load and save from an XML file. Sets error string and returns false on an error. */
    EXPORT void save(const std::string& filename);
    EXPORT int load(const std::string& filename, bool clear = true);
#endif

    EXPORT PlanetsUniverse();

    /* Advance the universe by the specified amount of time. */
    EXPORT void advance(float time);

    inline bool isEmpty() const { return planets.size() == 0; }
    /* As size_t is unsigned, any keys less than the universe size are valid and any others are not. */
    inline bool isValid(const key_type& key) const { return key < planets.size(); }
    inline Planet& operator [] (const key_type& key) { return planets.at(key); }
    EXPORT void remove(const key_type key, const key_type replacement = -1);

    /* Is a planet selected? */
    inline bool isSelectedValid() const { return isValid(selected); }
    /* Get the currently selected planet. Don't call without checking for validity first. */
    inline Planet& getSelected() { return planets[selected]; }
    /* Deselect the currently selected planet. */
    inline void resetSelected() { selected = -1; }

    /* Get a key for a random planet in the list. Will always retun a valid key unless universe is empty. */
    EXPORT key_type getRandomPlanet();

    /* Iterators and stuff. */
    inline iterator begin() { return planets.begin(); }
    inline iterator end() { return planets.end(); }
    inline const_iterator cbegin() const { return planets.cbegin(); }
    inline const_iterator cend() const { return planets.cend(); }
    inline list_type::size_type size() const { return planets.size(); }

    inline void randSeed(unsigned int seed) { generator.seed(seed); }

    /* Make the weighted average position and velocity of all planets 0.
     * After this if all the planets merged into one it would be stationary at the origin. */
    EXPORT void centerAll();

    /* Functions for destroying stuff. */
    inline void deleteAll() { planets.clear(); resetSelected(); }
    EXPORT void deleteEscapees();
    inline void deleteSelected() { if (isSelectedValid()) remove(selected); }
};
