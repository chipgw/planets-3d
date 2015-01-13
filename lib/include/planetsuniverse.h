#pragma once

#include "planet.h"
#include <map>
#include <random>
#include <string>
#include <cstdint>
#include <glm/mat4x4.hpp>

class PlanetsUniverse {
public:
    typedef uint32_t key_type;
    typedef std::map<key_type, Planet> list_type;
    typedef list_type::iterator iterator;
    typedef list_type::const_iterator const_iterator;

private:
    list_type planets;

    std::string errorMsg;

    std::default_random_engine generator;

public:
    /* The gravity constant */
    static const float gravityconst;
    /* The factor for apparent velocity.
     * (UI velocity * this = actual velocity, because it would be really really small if done right.) */
    static const float velocityfac;

    /* UI limits on planet size. */
    static const float min_mass;
    static const float max_mass;

    key_type selected;

    /* Speed multiplier for simulation. */
    float simspeed;
    /* How many sub-steps to perform per frame for better accuracy. */
    int stepsPerFrame;

    /* Make new planets. */
    key_type addPlanet(const Planet &planet, key_type keyHint = 0);
    void generateRandom(const int &count, const float &positionRange, const float &maxVelocity, const float &maxMass);
    key_type addOrbital(Planet &around, const float &radius, const float &mass, const glm::mat4 &plane);
    void generateRandomOrbital(const int &count, key_type target);

    /* Load and save from an XML file. Sets error string and returns false on an error. */
    bool save(const std::string& filename);
    bool load(const std::string& filename, bool clear = true);

    PlanetsUniverse();

    /* Advance the universe by the specified amount of time. */
    void advance(float time);

    inline bool isEmpty() const { return planets.size() == 0; }
    inline bool isValid(const key_type &key) const { return planets.count(key) > 0; }
    inline void remove(const key_type &key) { planets.erase(key); }
    inline Planet &operator [] (const key_type &key) { return planets[key]; }

    /* Is a planet selected? */
    inline bool isSelectedValid() const { return planets.count(selected) > 0; }
    /* Get the currently selected planet. Don't call without checking for validity first. */
    inline Planet &getSelected() { return planets[selected]; }
    /* Deselect the currently selected planet. */
    inline void resetSelected() { selected = 0; }

    /* Iterators and stuff. */
    inline iterator begin() { return planets.begin(); }
    inline iterator end() { return planets.end(); }
    inline const_iterator cbegin() const { return planets.cbegin(); }
    inline const_iterator cend() const { return planets.cend(); }
    inline const_iterator find(const key_type &key) { return planets.find(key); }
    inline list_type::size_type size() const { return planets.size(); }

    /* If loading or saving had an error, this will say what it was. */
    inline std::string getErrorMessage() const { return errorMsg; }

    /* Make the weighted average position and velocity of all planets 0.
     * After this if all the planets merged into one it would be stationary at the origin. */
    void centerAll();

    /* Functions for destroying stuff. */
    void deleteAll() { planets.clear(); resetSelected(); }
    void deleteEscapees();
    void deleteSelected() { if(isSelectedValid()) planets.erase(selected); }
};
