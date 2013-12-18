#include "include/planetsuniverse.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>
#include <QColor>
#include <QMessageBox>
#include <qmath.h>

PlanetsUniverse::PlanetsUniverse() : selected(0), simspeed(1.0f), stepsPerFrame(20) {}

bool PlanetsUniverse::load(const QString &filename){
    if(!QFile::exists(filename)){
        QMessageBox::warning(NULL, tr("Error loading simulation!"), tr("file \"%1\" does not exist!").arg(filename));
        return false;
    }

    QFile file(filename);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QMessageBox::warning(NULL, tr("Error loading simulation!"), tr("Unable to open file \"%1\" for reading!").arg(filename));
        return false;
    }

    QXmlStreamReader xml(&file);

    if(!(xml.readNextStartElement() && xml.name() == "planets-3d-universe")){
        QMessageBox::warning(NULL, tr("Error loading simulation!"), tr("\"%1\" is not a valid universe file!").arg(filename));
        return false;
    }

    deleteAll();
    while(xml.readNextStartElement()) {
        if(xml.name() == "planet"){
            Planet planet;

#if QT_VERSION >= 0x050100
            planet.setMass(xml.attributes().value("mass").toFloat());
#else
            planet.setMass(xml.attributes().value("mass").toString().toFloat());
#endif

            QRgb color = QColor(xml.attributes().value("color").toString()).rgb();

            while(xml.readNextStartElement()){
                if(xml.name() == "position"){
#if QT_VERSION >= 0x050100
                    planet.position.setX(xml.attributes().value("x").toFloat());
                    planet.position.setY(xml.attributes().value("y").toFloat());
                    planet.position.setZ(xml.attributes().value("z").toFloat());
#else
                    planet.position.setX(xml.attributes().value("x").toString().toFloat());
                    planet.position.setY(xml.attributes().value("y").toString().toFloat());
                    planet.position.setZ(xml.attributes().value("z").toString().toFloat());
#endif
                    xml.readNext();
                }else if(xml.name() == "velocity"){
#if QT_VERSION >= 0x050100
                    planet.velocity.setX(xml.attributes().value("x").toFloat() * velocityfac);
                    planet.velocity.setY(xml.attributes().value("y").toFloat() * velocityfac);
                    planet.velocity.setZ(xml.attributes().value("z").toFloat() * velocityfac);
#else
                    planet.velocity.setX(xml.attributes().value("x").toString().toFloat() * velocityfac);
                    planet.velocity.setY(xml.attributes().value("y").toString().toFloat() * velocityfac);
                    planet.velocity.setZ(xml.attributes().value("z").toString().toFloat() * velocityfac);
#endif
                    xml.readNext();
                }
            }
            addPlanet(planet, color);
            xml.readNext();
        }
    }

    if(xml.hasError()){
        deleteAll();
        QMessageBox::warning(NULL, tr("Error loading simulation!"), tr("Error in file \"%1\": %2").arg(filename).arg(xml.errorString()));
        return false;
    }
    sizeChanged();

    return true;
}

bool PlanetsUniverse::save(const QString &filename){
    QFile file(filename);

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox::warning(NULL, tr("Error saving simulation!"), tr("Unable to open file \"%1\" for writing!").arg(filename));
        return false;
    }

    QXmlStreamWriter xml(&file);

    xml.setAutoFormatting(true);

    xml.writeStartDocument();
    xml.writeStartElement("planets-3d-universe");

    for(const_iterator i = planets.constBegin(); i != planets.constEnd(); ++i){
        const Planet &planet = i.value();

        xml.writeStartElement("planet"); {
            xml.writeAttribute("mass", QString::number(planet.mass()));

            xml.writeAttribute("color", QColor(i.key() & RGB_MASK).name());

            xml.writeStartElement("position"); {
                xml.writeAttribute("x", QString::number(planet.position.x()));
                xml.writeAttribute("y", QString::number(planet.position.y()));
                xml.writeAttribute("z", QString::number(planet.position.z()));
            } xml.writeEndElement();

            xml.writeStartElement("velocity"); {
                xml.writeAttribute("x", QString::number(planet.velocity.x() / velocityfac));
                xml.writeAttribute("y", QString::number(planet.velocity.y() / velocityfac));
                xml.writeAttribute("z", QString::number(planet.velocity.z() / velocityfac));
            } xml.writeEndElement();
        } xml.writeEndElement();
    }
    xml.writeEndElement();

    xml.writeEndDocument();

    if(xml.hasError()){
        QMessageBox::warning(NULL, tr("Error saving simulation!"), tr("Error writing to file \"%1\"!").arg(filename));
        return false;
    }

    return true;
}

void PlanetsUniverse::advance(float time){
    int planetcount = planets.size();
    time *= simspeed;
    time /= stepsPerFrame;

    for(int s = 0; s < stepsPerFrame; ++s){
        for(planet_iterator i = planets.begin(); i != planets.end();){
            Planet &planet = i.value();

            if(planet.mass() <= 0.0f){
                i = planets.erase(i);
            }else{
                for(planet_iterator o = i + 1; o != planets.end();){
                    Planet &other = o.value();

                    QVector3D direction = other.position - planet.position;
                    float distancesqr = direction.lengthSquared();

                    if(distancesqr < pow((planet.radius() + other.radius()) * 0.8f, 2)){
                        planet.position = other.position * other.mass() + planet.position * planet.mass();
                        planet.velocity = other.velocity * other.mass() + planet.velocity * planet.mass();
                        planet.setMass(planet.mass() + other.mass());
                        planet.position /= planet.mass();
                        planet.velocity /= planet.mass();
                        if(o.key() == selected){
                            selected = i.key();
                        }
                        planet.path.clear();
                        o = planets.erase(o);
                    }else{
                        direction.normalize();
                        direction *= gravityconst * ((other.mass() * planet.mass()) / distancesqr) * time;

                        planet.velocity += direction / planet.mass();
                        other.velocity -= direction / other.mass();
                        ++o;
                    }
                }

                planet.position += planet.velocity * time;
                planet.updatePath();

                ++i;
            }
        }
    }
    if(planetcount != planets.size()){
        sizeChanged();
    }
}

QRgb PlanetsUniverse::addPlanet(const Planet &planet, QRgb colorhint){
    colorhint |= ~RGB_MASK;

    while(planets.contains(colorhint) || (colorhint & RGB_MASK) == 0){
        colorhint = qrand() | ~RGB_MASK;
    }

    planets[colorhint] = planet;
    sizeChanged();
    return colorhint;
}

void PlanetsUniverse::generateRandom(const int &count, const float &range, const float &velocity, const float &mass){
    for(int i = 0; i < count; ++i){
        addPlanet(Planet(QVector3D(float(qrand()) / RAND_MAX - 0.5f, float(qrand()) / RAND_MAX - 0.5f, float(qrand()) / RAND_MAX - 0.5f) * range,
                         QVector3D(float(qrand()) / RAND_MAX - 0.5f, float(qrand()) / RAND_MAX - 0.5f, float(qrand()) / RAND_MAX - 0.5f) * velocity,
                         (float(qrand()) / RAND_MAX) * mass));
    }
}

void PlanetsUniverse::deleteAll(){
    planets.clear();
    selected = 0;
    sizeChanged();
}

void PlanetsUniverse::deleteEscapees(){
    int planetcount = planets.size();

    QVector3D averagePosition, averageVelocity;
    float totalmass = 0.0f;

    foreach(const Planet &planet, planets){
        averagePosition += planet.position * planet.mass();
        averageVelocity += planet.velocity * planet.mass();
        totalmass += planet.mass();
    }

    averagePosition /= totalmass;
    averageVelocity /= totalmass;

    float limits = 1.0e8f;

    for(planet_iterator i = planets.begin(); i != planets.end();){
        if((i.value().position - averagePosition).lengthSquared() > limits){
            i = planets.erase(i);
        } else{
            ++i;
        }
    }

    if(planetcount != planets.size()){
        sizeChanged();
    }
}

void PlanetsUniverse::deleteSelected(){
    planets.remove(selected);
    sizeChanged();
}

void PlanetsUniverse::centerAll(){
    QVector3D averagePosition, averageVelocity;
    float totalmass = 0.0f;

    foreach(const Planet &planet, planets){
        averagePosition += planet.position * planet.mass();
        averageVelocity += planet.velocity * planet.mass();
        totalmass += planet.mass();
    }

    averagePosition /= totalmass;
    averageVelocity /= totalmass;

    if(!averagePosition.isNull() || !averageVelocity.isNull()){
        for(planet_iterator i = planets.begin(); i != planets.end(); ++i){
            Planet &planet = i.value();

            planet.position -= averagePosition;
            planet.velocity -= averageVelocity;
            planet.path.clear();
        }
    }
}

bool PlanetsUniverse::isEmpty(){
    return planets.isEmpty();
}

bool PlanetsUniverse::isValid(const QRgb &key){
    return planets.contains(key);
}

void PlanetsUniverse::remove(const QRgb &key){
    planets.remove(key);
    sizeChanged();
}

Planet &PlanetsUniverse::operator [] (const QRgb &key){
    return planets[key];
}

bool PlanetsUniverse::isSelectedValid(){
    return planets.contains(selected);
}

Planet &PlanetsUniverse::getSelected(){
    return planets[selected];
}

PlanetsUniverse::const_iterator PlanetsUniverse::begin(){
    return planets.constBegin();
}

PlanetsUniverse::const_iterator PlanetsUniverse::end(){
    return planets.constEnd();
}

int PlanetsUniverse::size(){
    return planets.size();
}

void PlanetsUniverse::sizeChanged(){
    if(planets.size() == 1){
        emit updatePlanetCountMessage(tr("1 planet"));
    }else{
        emit updatePlanetCountMessage(tr("%1 planets").arg(planets.size()));
    }
}

const float PlanetsUniverse::gravityconst = 6.67e-11f;

const float PlanetsUniverse::velocityfac = 1.0e-5f;
