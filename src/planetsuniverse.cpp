#include "include/planetsuniverse.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>

PlanetsUniverse::PlanetsUniverse(){
    selected = 0;
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
                            planet.position.setX(xml.attributes().value("x").toString().toFloat());
                            planet.position.setY(xml.attributes().value("y").toString().toFloat());
                            planet.position.setZ(xml.attributes().value("z").toString().toFloat());
                            xml.readNext();
                        }
                        if(xml.name() == "velocity"){
                            planet.velocity.setX(xml.attributes().value("x").toString().toFloat() * velocityfac);
                            planet.velocity.setY(xml.attributes().value("y").toString().toFloat() * velocityfac);
                            planet.velocity.setZ(xml.attributes().value("z").toString().toFloat() * velocityfac);
                            xml.readNext();
                        }
                    }
                    createPlanet(planet.position, planet.velocity, planet.mass);
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

    QMutableMapIterator<QRgb, Planet> i(planets);
    while (i.hasNext()) {
        const Planet &planet = i.next().value();

        xml.writeStartElement("planet");

        xml.writeAttribute("mass", QString::number(planet.mass));

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
        for(QMutableMapIterator<QRgb, Planet> i(planets); i.hasNext();){
            Planet &planet = i.next().value();
            QRgb planetkey = i.key();

            while(i.hasNext()){
                Planet &other = i.next().value();

                if(&other == &planet){
                    continue;
                }
                else{
                    QVector3D direction = other.position - planet.position;
                    float distance = direction.lengthSquared();

                    if(sqrt(distance) < (planet.getRadius() + other.getRadius()) * 0.8f){
                        planet.position = (other.position * other.mass + planet.position * planet.mass) / (other.mass + planet.mass);
                        planet.velocity = (other.velocity * other.mass + planet.velocity * planet.mass) / (other.mass + planet.mass);
                        planet.mass += other.mass;
                        if(i.key() == selected){
                            selected = planetkey;
                        }
                        i.remove();
                        planet.path.clear();
                    }else{
                        float frc = gravityconst * ((other.mass * planet.mass) / distance);

                        planet.velocity += direction * frc * time / planet.mass;
                        other.velocity -= direction * frc * time / other.mass;
                    }
                }
            }
            i.toFront();
            while(i.hasNext() && i.next().key() != planetkey);

            planet.position += planet.velocity * time;
            planet.updatePath();

        }
    }
}

QRgb PlanetsUniverse::createPlanet(QVector3D position, QVector3D velocity, float mass){
    Planet planet;
    planet.position = position;
    planet.velocity = velocity;
    planet.mass = mass;
    QRgb value = qrand() | 0xff000000;
    while(planets.contains(value)){
        value = qrand() | 0xff000000;
    }
    planets[value] = planet;
    return value;
}

void PlanetsUniverse::deleteAll(){
    planets.clear();
    selected = 0;
}

void PlanetsUniverse::centerAll(){
    QMutableMapIterator<QRgb, Planet> i(planets);
    QVector3D averagePosition, averageVelocity;
    float totalmass = 0.0f;
    while (i.hasNext()) {
        Planet &planet = i.next().value();

        averagePosition += planet.position * planet.mass;
        averageVelocity += planet.velocity * planet.mass;
        totalmass += planet.mass;
    }
    averagePosition /= totalmass;
    averageVelocity /= totalmass;

    i.toFront();
    while (i.hasNext()) {
        Planet &planet = i.next().value();

        planet.position -= averagePosition;
        planet.velocity -= averageVelocity;
        planet.path.clear();
    }
}
