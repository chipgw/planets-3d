#pragma once

#include "types.h"
#include <map>
#include <random>
#include <string>
#include <glm/mat4x4.hpp>

class PlanetsUniverse {
public:
    typedef std::map<key_type, Planet> list_type;
    typedef list_type::iterator iterator;
    typedef list_type::const_iterator const_iterator;

private:
    list_type planets;

    std::default_random_engine generator;

public:
    /* The gravity constant */
    const float gravityconst;
    /* The factor for apparent velocity.
     * (UI velocity * this = actual velocity, because it would be really really small if done right.) */
    const float velocityfac;

    /* UI limits on planet size. */
    const float min_mass;
    const float max_mass;

    std::vector<glm::vec3>::size_type pathLength;
    float pathRecordDistance;

    key_type selected, following;

    /* Speed multiplier for simulation. */
    float simspeed;
    /* How many sub-steps to perform per frame for better accuracy. */
    int stepsPerFrame;

    /* Make new planets. */
    EXPORT key_type addPlanet(const Planet &planet, key_type keyHint = 0);
    EXPORT void generateRandom(const int &count, const float &positionRange, const float &maxVelocity, const float &maxMass);
    EXPORT key_type addOrbital(Planet &around, const float &radius, const float &mass, const glm::mat4 &plane);
    EXPORT void generateRandomOrbital(const int &count, key_type target);

    /* Load and save from an XML file. Sets error string and returns false on an error. */
    EXPORT bool save(const std::string& filename, std::string& errorMsg);
    EXPORT bool load(const std::string& filename, std::string& errorMsg, bool clear = true);

    EXPORT PlanetsUniverse();

    /* Advance the universe by the specified amount of time. */
    EXPORT void advance(float time);

    inline bool isEmpty() const { return planets.size() == 0; }
    inline bool isValid(const key_type &key) const { return planets.count(key) > 0; }
    inline void remove(const key_type &key) { planets.erase(key); }
    inline Planet &operator [] (const key_type &key) { return planets.at(key); }

    /* Is a planet selected? */
    inline bool isSelectedValid() const { return planets.count(selected) > 0; }
    /* Get the currently selected planet. Don't call without checking for validity first. */
    inline Planet &getSelected() { return planets.at(selected); }
    /* Deselect the currently selected planet. */
    inline void resetSelected() { selected = 0; }

    /* Iterators and stuff. */
    inline iterator begin() { return planets.begin(); }
    inline iterator end() { return planets.end(); }
    inline const_iterator cbegin() const { return planets.cbegin(); }
    inline const_iterator cend() const { return planets.cend(); }
    inline const_iterator find(const key_type &key) { return planets.find(key); }
    inline list_type::size_type size() const { return planets.size(); }

    /* Make the weighted average position and velocity of all planets 0.
     * After this if all the planets merged into one it would be stationary at the origin. */
    EXPORT void centerAll();

    /* Functions for destroying stuff. */
    inline void deleteAll() { planets.clear(); resetSelected(); }
    EXPORT void deleteEscapees();
    inline void deleteSelected() { if(isSelectedValid()) planets.erase(selected); }
};
