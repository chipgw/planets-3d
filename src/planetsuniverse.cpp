#include "include/planetsuniverse.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>
#include <QColor>
#include <QMessageBox>
#include <qmath.h>

#define ALPHAMASK 0xff000000

PlanetsUniverse::PlanetsUniverse() : selected(0), simspeed(1.0f), stepsPerFrame(100) {}

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

    for(QMapIterator<QRgb, Planet> i(planets_p); i.hasNext();) {
        const Planet &planet = i.next().value();

        xml.writeStartElement("planet"); {
            xml.writeAttribute("mass", QString::number(planet.mass()));

            xml.writeAttribute("color", QColor(i.key() ^ALPHAMASK).name());

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
    int planetcount = planets_p.size();
    time *= simspeed;
    time /= stepsPerFrame;

    for(int s = 0; s < stepsPerFrame; s++){
        for(QMutableMapIterator<QRgb, Planet> i(planets_p); i.hasNext();){
            Planet &planet = i.next().value();
            QRgb planetkey = i.key();

            if(planet.mass() <= 0.0f){
                i.remove();
                continue;
            }

            while(i.hasNext()){
                Planet &other = i.next().value();

                QVector3D direction = other.position - planet.position;
                float distance = direction.lengthSquared();

                if(sqrt(distance) < (planet.radius() + other.radius()) * 0.8f){
                    planet.position = (other.position * other.mass() + planet.position * planet.mass()) / (other.mass() + planet.mass());
                    planet.velocity = (other.velocity * other.mass() + planet.velocity * planet.mass()) / (other.mass() + planet.mass());
                    planet.setMass(planet.mass() + other.mass());
                    if(i.key() == selected){
                        selected = planetkey;
                    }
                    i.remove();
                    planet.path.clear();
                }else{
                    float frc = gravityconst * ((other.mass() * planet.mass()) / distance);

                    direction.normalize();
                    planet.velocity += direction * frc * time / planet.mass();
                    other.velocity -= direction * frc * time / other.mass();
                }
            }

            i.toFront();
            while(i.hasNext() && i.next().key() != planetkey);

            planet.position += planet.velocity * time;
            planet.updatePath();
        }
    }
    if(planetcount != planets_p.size()){
        sizeChanged();
    }
}

QRgb PlanetsUniverse::addPlanet(const Planet &planet, QRgb colorhint){
    if(colorhint != 0){
        colorhint = colorhint | ALPHAMASK;
    }else{
        colorhint = qrand() | ALPHAMASK;
    }

    while(planets_p.contains(colorhint)){
        colorhint = qrand() | ALPHAMASK;
    }

    planets_p[colorhint] = planet;
    sizeChanged();
    return colorhint;
}

void PlanetsUniverse::deleteAll(){
    planets_p.clear();
    selected = 0;
    sizeChanged();
}

void PlanetsUniverse::centerAll(){
    QVector3D averagePosition, averageVelocity;
    float totalmass = 0.0f;

    foreach(const Planet &planet, planets_p){
        averagePosition += planet.position * planet.mass();
        averageVelocity += planet.velocity * planet.mass();
        totalmass += planet.mass();
    }

    averagePosition /= totalmass;
    averageVelocity /= totalmass;

    for(QMutableMapIterator<QRgb, Planet> i(planets_p); i.hasNext();) {
        Planet &planet = i.next().value();

        planet.position -= averagePosition;
        planet.velocity -= averageVelocity;
        planet.path.clear();
    }
}

bool PlanetsUniverse::isEmpty(){
    return planets_p.isEmpty();
}

bool PlanetsUniverse::isValid(QRgb key){
    return planets_p.contains(key);
}

void PlanetsUniverse::remove(QRgb key){
    planets_p.remove(key);
    sizeChanged();
}

Planet &PlanetsUniverse::operator [] (QRgb key){
    return planets_p[key];
}

const QMap<QRgb, Planet> &PlanetsUniverse::planets(){
    return planets_p;
}

void PlanetsUniverse::sizeChanged(){
    if(planets_p.size() == 1){
        emit updatePlanetCountMessage(tr("1 planet"));
    }else{
        emit updatePlanetCountMessage(tr("%1 planets").arg(planets_p.size()));
    }
}
