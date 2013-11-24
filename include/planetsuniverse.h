#ifndef PLANETSUNIVERSE_H
#define PLANETSUNIVERSE_H

#include "planet.h"
#include <QRgb>
#include <QObject>
#include <QMap>

// the gravity constant
const float gravityconst = 6.67e-11f;
// the factor for apparent velocity. (i.e. UI velocity * this = actual velocity, because it would be really really small if done right.)
const float velocityfac = 1.0e-5f;

class PlanetsUniverse : public QObject {
    Q_OBJECT
private:
    QMap<QRgb, Planet> planets_p;
    void sizeChanged();

public:
    QRgb selected;

    float simspeed;
    int stepsPerFrame;

    QRgb addPlanet(const Planet &planet, QRgb colorhint = 0);
    void generateRandom(const int &count);

    bool save(const QString& filename);
    bool load(const QString& filename);

    PlanetsUniverse();

    void advance(float time);

    bool isEmpty();
    bool isValid(const QRgb &key);
    void remove(const QRgb &key);
    Planet &operator [] (const QRgb &key);

    QMap<QRgb, Planet>::const_iterator begin();
    QMap<QRgb, Planet>::const_iterator end();
    int size();

public slots:
    void centerAll();
    void deleteAll();

signals:
    void updatePlanetCountMessage(const QString &text);
};

#endif // PLANETSUNIVERSE_H
