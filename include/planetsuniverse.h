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
    bool load(const QString& filename);

    PlanetsUniverse();

    void advance(float time);

    bool isEmpty();
    bool isValid(const QRgb &key);
    void remove(const QRgb &key);
    Planet &operator [] (const QRgb &key);
    bool isSelectedValid();
    Planet &getSelected();

    typedef QMap<QRgb, Planet>::const_iterator const_iterator;
    const_iterator begin();
    const_iterator end();
    int size();

public slots:
    void centerAll();
    void deleteAll();
    void deleteEscapees();
    void deleteSelected();

signals:
    void updatePlanetCountMessage(const QString &text);
};

#endif // PLANETSUNIVERSE_H
