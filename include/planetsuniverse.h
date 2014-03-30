#ifndef PLANETSUNIVERSE_H
#define PLANETSUNIVERSE_H

#include "planet.h"
#include <QRgb>
#include <QObject>
#include <QMap>


class PlanetsUniverse : public QObject {
    Q_OBJECT
private:
    QMap<QRgb, Planet> planets;
    void sizeChanged();
    typedef QMap<QRgb, Planet>::iterator planet_iterator;

public:
    // the gravity constant
    static const float gravityconst;
    // the factor for apparent velocity. (i.e. UI velocity * this = actual velocity, because it would be really really small if done right.)
    static const float velocityfac;

    QRgb selected;

    float simspeed;
    int stepsPerFrame;

    QRgb addPlanet(const Planet &planet, QRgb colorhint = 0);
    void generateRandom(const int &count, const float &range, const float &velocity, const float &mass);

    bool save(const QString& filename);
    bool load(const QString& filename, bool clear = true);

    PlanetsUniverse();

    void advance(float time);

    inline bool isEmpty() { return planets.isEmpty(); }
    inline bool isValid(const QRgb &key) { return planets.contains(key); }
    inline void remove(const QRgb &key) { planets.remove(key); sizeChanged(); }
    inline Planet &operator [] (const QRgb &key) { return planets[key]; }

    inline bool isSelectedValid() { return planets.contains(selected); }
    inline Planet &getSelected() { return planets[selected]; }

    typedef QMap<QRgb, Planet>::const_iterator const_iterator;
    inline const_iterator begin() { return planets.constBegin(); }
    inline const_iterator end() { return planets.constEnd(); }
    inline const_iterator find(const QRgb &key) { return planets.find(key); }
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
