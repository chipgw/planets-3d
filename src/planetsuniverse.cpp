#include "include/planetsuniverse.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>
#include <QColor>
#include <QMessageBox>
#include <qmath.h>
#include <chrono>

using std::uniform_int_distribution;
using std::uniform_real_distribution;

PlanetsUniverse::PlanetsUniverse() : selected(0), simspeed(1.0f), stepsPerFrame(20), generator(std::chrono::system_clock::now().time_since_epoch().count()) {}

bool PlanetsUniverse::load(const QString &filename, bool clear){
    QFile file(filename);

    if(!file.exists()){
        errorMsg = tr("file \"%1\" does not exist!").arg(filename);
        return false;
    }

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        errorMsg = tr("Unable to open file \"%1\" for reading!").arg(filename);
        return false;
    }

    QXmlStreamReader xml(&file);

    if(!(xml.readNextStartElement() && xml.name() == "planets-3d-universe")){
        errorMsg = tr("\"%1\" is not a valid universe file!").arg(filename);
        return false;
    }

#if QT_VERSION >= 0x050100
#define GETATTRIBUTE(A) xml.attributes().value(A).toFloat()
#else
#define GETATTRIBUTE(A) xml.attributes().value(A).toString().toFloat()
#endif

    if(clear){
        deleteAll();
    }

    while(xml.readNextStartElement()) {
        if(xml.name() == "planet"){
            Planet planet;
            planet.setMass(GETATTRIBUTE("mass"));

            key_type color = QColor(xml.attributes().value("color").toString()).rgb();

            while(xml.readNextStartElement()){
                if(xml.name() == "position"){
                    planet.position.setX(GETATTRIBUTE("x"));
                    planet.position.setY(GETATTRIBUTE("y"));
                    planet.position.setZ(GETATTRIBUTE("z"));
                    xml.readNext();
                }else if(xml.name() == "velocity"){
                    planet.velocity.setX(GETATTRIBUTE("x") * velocityfac);
                    planet.velocity.setY(GETATTRIBUTE("y") * velocityfac);
                    planet.velocity.setZ(GETATTRIBUTE("z") * velocityfac);
                    xml.readNext();
                }
            }
            addPlanet(planet, color);
            xml.readNext();
        }
    }

#undef GETATTRIBUTE

    if(xml.hasError()){
        if(clear){
            deleteAll();
        }
        errorMsg = tr("Error in file \"%1\": %2").arg(filename).arg(xml.errorString());
        return false;
    }
    sizeChanged();

    return true;
}

bool PlanetsUniverse::save(const QString &filename){
    QFile file(filename);

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        errorMsg = tr("Unable to open file \"%1\" for writing!").arg(filename);
        return false;
    }

    QXmlStreamWriter xml(&file);

    xml.setAutoFormatting(true);

    xml.writeStartDocument();
    xml.writeStartElement("planets-3d-universe");

    for(const_iterator i = planets.cbegin(); i != planets.cend(); ++i){
        xml.writeStartElement("planet"); {
            xml.writeAttribute("mass", QString::number(i->second.mass()));

            xml.writeAttribute("color", QColor(i->first & RGB_MASK).name());

            xml.writeStartElement("position"); {
                xml.writeAttribute("x", QString::number(i->second.position.x()));
                xml.writeAttribute("y", QString::number(i->second.position.y()));
                xml.writeAttribute("z", QString::number(i->second.position.z()));
            } xml.writeEndElement();

            xml.writeStartElement("velocity"); {
                xml.writeAttribute("x", QString::number(i->second.velocity.x() / velocityfac));
                xml.writeAttribute("y", QString::number(i->second.velocity.y() / velocityfac));
                xml.writeAttribute("z", QString::number(i->second.velocity.z() / velocityfac));
            } xml.writeEndElement();
        } xml.writeEndElement();
    }
    xml.writeEndElement();

    xml.writeEndDocument();

    if(xml.hasError()){
        errorMsg = tr("Error writing to file \"%1\"!").arg(filename);
        return false;
    }

    return true;
}

void PlanetsUniverse::advance(float time){
    int planetcount = planets.size();
    time *= simspeed;
    time /= stepsPerFrame;

    for(int s = 0; s < stepsPerFrame; ++s){
        for(iterator i = planets.begin(); i != planets.end();){
            if(i->second.mass() <= 0.0f){
                i = planets.erase(i);
            }else{
                iterator o = i;
                ++o;
                for(;o != planets.end();){
                    QVector3D direction = o->second.position - i->second.position;
                    float distancesqr = direction.lengthSquared();

                    if(distancesqr < pow(i->second.radius() + o->second.radius(), 2)){
                        i->second.position = o->second.position * o->second.mass() + i->second.position * i->second.mass();
                        i->second.velocity = o->second.velocity * o->second.mass() + i->second.velocity * i->second.mass();
                        i->second.setMass(i->second.mass() + o->second.mass());
                        i->second.position /= i->second.mass();
                        i->second.velocity /= i->second.mass();
                        if(o->first == selected){
                            selected = i->first;
                        }
                        i->second.path.clear();
                        o = planets.erase(o);
                    }else{
                        direction.normalize();
                        direction *= gravityconst * ((o->second.mass() * i->second.mass()) / distancesqr) * time;

                        i->second.velocity += direction / i->second.mass();
                        o->second.velocity -= direction / o->second.mass();
                        ++o;
                    }
                }

                i->second.position += i->second.velocity * time;
                i->second.updatePath();

                ++i;
            }
        }
    }
    if(planetcount != planets.size()){
        sizeChanged();
    }
}

PlanetsUniverse::key_type PlanetsUniverse::addPlanet(const Planet &planet, key_type colorhint){
    uniform_int_distribution<QRgb> color_gen(0xFF000001, 0xFFFFFFFF);

    while(planets.count(colorhint) > 0 || (colorhint & RGB_MASK) == 0){
        colorhint = color_gen(generator);
    }

    planets[colorhint] = planet;
    sizeChanged();
    return colorhint;
}

void PlanetsUniverse::generateRandom(const int &count, const float &positionRange, const float &maxVelocity, const float &maxMass){
    uniform_real_distribution<float> position(-positionRange, positionRange);
    uniform_real_distribution<float> velocity(-maxVelocity, maxVelocity);
    uniform_real_distribution<float> mass(min_mass, maxMass);

    for(int i = 0; i < count; ++i){
        addPlanet(Planet(QVector3D(position(generator), position(generator), position(generator)),
                         QVector3D(velocity(generator), velocity(generator), velocity(generator)),
                         mass(generator)));
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

    for(const_iterator i = planets.cbegin(); i != planets.cend(); ++i){
        averagePosition += i->second.position * i->second.mass();
        averageVelocity += i->second.velocity * i->second.mass();
        totalmass += i->second.mass();
    }

    averagePosition /= totalmass;
    averageVelocity /= totalmass;

    float limits = 1.0e12f;

    for(iterator i = planets.begin(); i != planets.end();){
        if((i->second.position - averagePosition).lengthSquared() > limits){
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
    if(isSelectedValid()){
        planets.erase(selected);
        sizeChanged();
    }
}

void PlanetsUniverse::centerAll(){
    QVector3D averagePosition, averageVelocity;
    float totalmass = 0.0f;

    for(const_iterator i = planets.cbegin(); i != planets.cend(); ++i){
        averagePosition += i->second.position * i->second.mass();
        averageVelocity += i->second.velocity * i->second.mass();
        totalmass += i->second.mass();
    }

    averagePosition /= totalmass;
    averageVelocity /= totalmass;

    if(!averagePosition.isNull() || !averageVelocity.isNull()){
        for(iterator i = planets.begin(); i != planets.end(); ++i){
            i->second.position -= averagePosition;
            i->second.velocity -= averageVelocity;
            i->second.path.clear();
        }
    }
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

const float PlanetsUniverse::min_mass = 1.0f;
const float PlanetsUniverse::max_mass = 1.0e9f;
