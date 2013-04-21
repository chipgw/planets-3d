#include "include/planetsuniverse.h"
#include <glm/gtx/norm.hpp>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>

PlanetsUniverse::PlanetsUniverse(){
    selected = NULL;
    simspeed = 1.0f;
}

bool PlanetsUniverse::load(const QString &filename){
    if(!QFile::exists(filename)){
        qDebug(qPrintable(tr("\"%1\" does not exist!").arg(filename)));
        return false;
    }
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return false;
    }

    QXmlStreamReader xml(&file);

    if(xml.readNextStartElement()) {
        if (xml.name() == "planets-3d-universe"){
            deleteAll();
            while(xml.readNextStartElement()) {
                if(xml.name() == "planet"){
                    Planet planet;

                    planet.mass = xml.attributes().value("mass").toString().toFloat();

                    while(xml.readNextStartElement()){
                        if(xml.name() == "position"){
                            planet.position.x = xml.attributes().value("x").toString().toFloat();
                            planet.position.y = xml.attributes().value("y").toString().toFloat();
                            planet.position.z = xml.attributes().value("z").toString().toFloat();
                            xml.readNext();
                        }
                        if(xml.name() == "velocity"){
                            planet.velocity.x = xml.attributes().value("x").toString().toFloat() * velocityfac;
                            planet.velocity.y = xml.attributes().value("y").toString().toFloat() * velocityfac;
                            planet.velocity.z = xml.attributes().value("z").toString().toFloat() * velocityfac;
                            xml.readNext();
                        }
                    }
                    planets.append(planet);
                    xml.readNext();
                }
            }
            if(xml.hasError()){
                qDebug(qPrintable(tr("\"%1\" had error: %2").arg(filename).arg(xml.errorString())));
                return false;
            }
        }

        else{
            qDebug(qPrintable(tr("\"%1\" is not a valid universe file!").arg(filename)));
            return false;
        }
    }


    return true;
}

bool PlanetsUniverse::save(const QString &filename){
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        return false;
    }

    QXmlStreamWriter xml(&file);

    xml.setAutoFormatting(true);

    xml.writeStartDocument();
    xml.writeStartElement("planets-3d-universe");

    QMutableListIterator<Planet> i(planets);
    while (i.hasNext()) {
        const Planet &planet = i.next();

        xml.writeStartElement("planet");

        xml.writeAttribute("mass", QString::number(planet.mass));

        xml.writeStartElement("position"); {
            xml.writeAttribute("x", QString::number(planet.position.x));
            xml.writeAttribute("y", QString::number(planet.position.y));
            xml.writeAttribute("z", QString::number(planet.position.z));
        } xml.writeEndElement();

        xml.writeStartElement("velocity"); {
            xml.writeAttribute("x", QString::number(planet.velocity.x / velocityfac));
            xml.writeAttribute("y", QString::number(planet.velocity.y / velocityfac));
            xml.writeAttribute("z", QString::number(planet.velocity.z / velocityfac));
        } xml.writeEndElement();

        xml.writeEndElement();
    }
    xml.writeEndElement();

    xml.writeEndDocument();

    return true;
}

void PlanetsUniverse::advance(float time, int steps){
    time *= simspeed;
    time /= steps;

    for(int s = 0; s < steps; s++){
        QMutableListIterator<Planet> i(planets);
        while (i.hasNext()) {
            Planet &planet = i.next();
            QMutableListIterator<Planet> o(i);
            while (o.hasNext()) {
                Planet &other = o.next();

                if(other == planet){
                    continue;
                }
                else{
                    glm::vec3 direction = other.position - planet.position;
                    float distance = glm::length2(direction);
                    float frc = gravityconst * ((other.mass * planet.mass) / distance);

                    planet.velocity += direction * frc * time / planet.mass;
                    other.velocity -= direction * frc * time / other.mass;

                    distance = sqrt(distance);

                    if(distance < planet.getRadius() + other.getRadius() / 2.0f){
                        planet.position = (other.position * other.mass + planet.position * planet.mass) / (other.mass + planet.mass);
                        planet.velocity = (other.velocity * other.mass + planet.velocity * planet.mass) / (other.mass + planet.mass);
                        planet.mass += other.mass;
                        if(&other == selected){
                            selected = &planet;
                        }
                        o.remove();
                        planet.path.clear();
                    }
                }
            }

            planet.position += planet.velocity * time;
            planet.updatePath();
        }
    }
}

Planet &PlanetsUniverse::createPlanet(glm::vec3 position, glm::vec3 velocity, float mass){
    Planet planet;
    planet.position = position;
    planet.velocity = velocity;
    planet.mass = mass;
    planets.append(planet);
    return planets.back();
}

void PlanetsUniverse::deleteAll(){
    QMutableListIterator<Planet> i(planets);
    while (i.hasNext()) {
        i.next();
        i.remove();
    }
    selected = NULL;
}

void PlanetsUniverse::centerAll(){
    QMutableListIterator<Planet> i(planets);
    glm::vec3 averagePosition(0.0f),averageVelocity(0.0f);
    float totalmass = 0.0f;
    while (i.hasNext()) {
        Planet &planet = i.next();

        averagePosition += planet.position * planet.mass;
        averageVelocity += planet.velocity * planet.mass;
        totalmass += planet.mass;
    }
    averagePosition /= totalmass;
    averageVelocity /= totalmass;

    i.toFront();
    while (i.hasNext()) {
        Planet &planet = i.next();

        planet.position -= averagePosition;
        planet.velocity -= averageVelocity;
        planet.path.clear();
    }
}
