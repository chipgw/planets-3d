#ifndef PLANETSUNIVERSE_H
#define PLANETSUNIVERSE_H

#include "planet.h"
#include <QRgb>
#include <QObject>
#include <QMap>


class PlanetsUniverse : public QObject {
    Q_OBJECT

public:
    typedef QRgb key_type;
    typedef QMap<key_type, Planet> list_type;
    typedef list_type::iterator iterator;
    typedef list_type::const_iterator const_iterator;

private:
    list_type planets;
    void sizeChanged();

public:
    // the gravity constant
    static const float gravityconst;
    // the factor for apparent velocity. (i.e. UI velocity * this = actual velocity, because it would be really really small if done right.)
    static const float velocityfac;

    key_type selected;

    float simspeed;
    int stepsPerFrame;

    key_type addPlanet(const Planet &planet, key_type colorhint = 0);
    void generateRandom(const int &count, const float &range, const float &velocity, const float &mass);

    bool save(const QString& filename);
    bool load(const QString& filename, bool clear = true);

    PlanetsUniverse();

    void advance(float time);

    inline bool isEmpty() { return planets.isEmpty(); }
    inline bool isValid(const key_type &key) { return planets.contains(key); }
    inline void remove(const key_type &key) { planets.remove(key); sizeChanged(); }
    inline Planet &operator [] (const key_type &key) { return planets[key]; }

    inline bool isSelectedValid() { return planets.contains(selected); }
    inline Planet &getSelected() { return planets[selected]; }

    inline const_iterator begin() { return planets.constBegin(); }
    inline const_iterator end() { return planets.constEnd(); }
    inline const_iterator find(const key_type &key) { return planets.find(key); }
    inline int size() { return planets.size(); }

public slots:
    void centerAll();
    void deleteAll();
    void deleteEscapees();
    void deleteSelected();

signals:
    void updatePlanetCountMessage(const QString &text);
};

#endif // PLANETSUNIVERSE_H
