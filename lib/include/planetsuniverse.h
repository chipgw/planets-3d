#pragma once

#include "planet.h"
#include <map>
#include <random>
#include <string>
#include <cstdint>

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
    /* the gravity constant */
    static const float gravityconst;
    /* the factor for apparent velocity. (i.e. UI velocity * this = actual velocity, because it would be really really small if done right.) */
    static const float velocityfac;

    /* UI limits on planet size. */
    static const float min_mass;
    static const float max_mass;

    key_type selected;

    float simspeed;
    int stepsPerFrame;

    key_type addPlanet(const Planet &planet, key_type colorhint = 0);
    void generateRandom(const int &count, const float &positionRange, const float &maxVelocity, const float &maxMass);

    bool save(const std::string& filename);
    bool load(const std::string& filename, bool clear = true);

    PlanetsUniverse();

    void advance(float time);

    inline bool isEmpty() const { return planets.size() == 0; }
    inline bool isValid(const key_type &key) const { return planets.count(key) > 0; }
    inline void remove(const key_type &key) { planets.erase(key); }
    inline Planet &operator [] (const key_type &key) { return planets[key]; }

    inline bool isSelectedValid() const { return planets.count(selected) > 0; }
    inline Planet &getSelected() { return planets[selected]; }
    inline void resetSelected() { selected = 0; }

    inline iterator begin() { return planets.begin(); }
    inline iterator end() { return planets.end(); }
    inline const_iterator cbegin() const { return planets.cbegin(); }
    inline const_iterator cend() const { return planets.cend(); }
    inline const_iterator find(const key_type &key) { return planets.find(key); }
    inline list_type::size_type size() const { return planets.size(); }

    inline std::string getErrorMessage() const { return errorMsg; }

    void centerAll();
    void deleteAll() { planets.clear(); resetSelected(); }
    void deleteEscapees();
    void deleteSelected() { if(isSelectedValid()) planets.erase(selected); }
};
