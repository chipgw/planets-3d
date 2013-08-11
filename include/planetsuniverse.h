#ifndef PLANETSUNIVERSE_H
#define PLANETSUNIVERSE_H

#include "planet.h"
#include <QRgb>
#include <QObject>
#include <QMap>

// the gravity constant
const float gravityconst = 6.67e-11f;
// the factor for apparent velocity. (i.e. UI velocity * this = actual velocity, because it would be really really small if done right.)
const float velocityfac = 1.0e-4f;

class PlanetsUniverse : public QObject {
    Q_OBJECT
public:
    QMap<QRgb, Planet> planets;
    QRgb selected;

    float simspeed;
    int stepsPerFrame;

    QRgb addPlanet(const Planet &planet, QRgb colorhint = 0);

    bool save(const QString& filename);
    bool load(const QString& filename);

    PlanetsUniverse();

    void advance(float time);

public slots:
    void centerAll();
    void deleteAll();
};

#endif // PLANETSUNIVERSE_H
