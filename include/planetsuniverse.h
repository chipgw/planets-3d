#ifndef PLANETSUNIVERSE_H
#define PLANETSUNIVERSE_H

#include "planet.h"
#include <QRgb>

class PlanetsUniverse : public QObject {
    Q_OBJECT
public:
    QMap<QRgb, Planet> planets;
    QRgb selected;

    float simspeed;

    QRgb addPlanet(const Planet &planet, QRgb colorhint = 0);

    bool save(const QString& filename);
    bool load(const QString& filename);

    PlanetsUniverse();

    void advance(float time, int steps);

public slots:
    void centerAll();
    void deleteAll();
};

#endif // PLANETSUNIVERSE_H
