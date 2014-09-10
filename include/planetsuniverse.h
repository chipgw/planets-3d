#ifndef PLANETSUNIVERSE_H
#define PLANETSUNIVERSE_H

#include "planet.h"
#include <QRgb>
#include <QObject>
#include <map>
#include <random>

class PlanetsUniverse : public QObject {
    Q_OBJECT

public:
    typedef QRgb key_type;
    typedef std::map<key_type, Planet> list_type;
    typedef list_type::iterator iterator;
    typedef list_type::const_iterator const_iterator;

private:
    list_type planets;

    QString errorMsg;

    std::default_random_engine generator;

public:
    // the gravity constant
    static const float gravityconst;
    // the factor for apparent velocity. (i.e. UI velocity * this = actual velocity, because it would be really really small if done right.)
    static const float velocityfac;

    // UI limits on planet size.
    static const float min_mass;
    static const float max_mass;

    key_type selected;

    float simspeed;
    int stepsPerFrame;

    key_type addPlanet(const Planet &planet, key_type colorhint = 0);
    void generateRandom(const int &count, const float &positionRange, const float &maxVelocity, const float &maxMass);

    bool save(const QString& filename);
    bool load(const QString& filename, bool clear = true);

    PlanetsUniverse();

    void advance(float time);

    inline bool isEmpty() { return planets.size() == 0; }
    inline bool isValid(const key_type &key) { return planets.count(key); }
    inline void remove(const key_type &key) { planets.erase(key); }
    inline Planet &operator [] (const key_type &key) { return planets[key]; }

    inline bool isSelectedValid() { return planets.count(selected); }
    inline Planet &getSelected() { return planets[selected]; }
    inline void resetSelected() { selected = 0; }

    inline iterator begin() { return planets.begin(); }
    inline iterator end() { return planets.end(); }
    inline const_iterator cbegin() { return planets.cbegin(); }
    inline const_iterator cend() { return planets.cend(); }
    inline const_iterator find(const key_type &key) { return planets.find(key); }
    inline int size() { return planets.size(); }

    inline QString getErrorMessage() { return errorMsg; }

public slots:
    void centerAll();
    void deleteAll();
    void deleteEscapees();
    void deleteSelected();
};

#endif // PLANETSUNIVERSE_H
