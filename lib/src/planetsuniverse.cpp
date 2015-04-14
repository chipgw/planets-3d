#include "planetsuniverse.h"
#include "planet.h"
#include <chrono>
#include <tinyxml.h>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/vector_query.hpp>
#include <glm/gtx/fast_square_root.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>
#include <cstdio>

#define RGB_MASK 0x00ffffff

using std::uniform_int_distribution;
using std::uniform_real_distribution;

PlanetsUniverse::PlanetsUniverse() : selected(0), simspeed(1.0f), stepsPerFrame(20),
    generator(std::chrono::system_clock::now().time_since_epoch().count()),
    gravityconst(6.67e-11f), velocityfac(1.0e-5f), min_mass(1.0f), max_mass(1.0e9f),
    pathLength(200), pathRecordDistance(0.25f) {}

bool PlanetsUniverse::load(const std::string &filename, std::string& errorMsg, bool clear){
    TiXmlDocument doc(filename);

    if(!doc.LoadFile()){
        errorMsg =  "Unable to load file \"" + filename + "\"!\n" + doc.ErrorDesc();
        return false;
    }

    TiXmlElement* root = doc.FirstChildElement("planets-3d-universe");
    if(root == nullptr){
        errorMsg = "\"" + filename + "\" is not a valid universe file!";
        return false;
    }

    if(clear){
        deleteAll();
    }

    for(TiXmlElement* element = root->FirstChildElement(); element != nullptr; element = element->NextSiblingElement()){
        if(element->ValueStr() == "planet"){
            Planet planet;
            planet.setMass(std::stof(element->Attribute("mass")));

            key_type color = 0;

            try{
                std::string colorStr = element->Attribute("color");
                colorStr.replace(0, 1, "");
                color = std::stoul(colorStr, nullptr, 16);
            } catch(std::exception) { /* Just ignore if it doesn't work. It isn't that critical. */ }

            for(TiXmlElement* sub = element->FirstChildElement(); sub != nullptr; sub = sub->NextSiblingElement()){
                if(sub->ValueStr() == "position"){
                    planet.position = glm::vec3(std::stof(sub->Attribute("x")),
                                                std::stof(sub->Attribute("y")),
                                                std::stof(sub->Attribute("z")));
                }else if(sub->ValueStr() == "velocity"){
                    planet.velocity = glm::vec3(std::stof(sub->Attribute("x")),
                                                std::stof(sub->Attribute("y")),
                                                std::stof(sub->Attribute("z"))) * velocityfac;
                }
            }
            addPlanet(planet, color);
        }
    }

    return true;
}

bool PlanetsUniverse::save(const std::string &filename, std::string& errorMsg){
    TiXmlDocument doc;

    doc.LinkEndChild(new TiXmlDeclaration("1.0", "", ""));

    TiXmlElement* root = new TiXmlElement("planets-3d-universe");

    for(const_iterator i = planets.cbegin(); i != planets.cend(); ++i){
        TiXmlElement* planet = new TiXmlElement("planet");
        planet->SetAttribute("mass", std::to_string(i->second.mass()));

        /* Puts uint32_t key into hexadecimal format #RRGGBB (No need to save alpha, as that's always 0xff) */
        const char* hex = "0123456789abcdef";
        planet->SetAttribute("color", std::string{'#',
                                                  hex[i->first >> 20 & 0xf],
                                                  hex[i->first >> 16 & 0xf],
                                                  hex[i->first >> 12 & 0xf],
                                                  hex[i->first >>  8 & 0xf],
                                                  hex[i->first >>  4 & 0xf],
                                                  hex[i->first       & 0xf]});

        TiXmlElement* position = new TiXmlElement("position");
        position->SetAttribute("x", std::to_string(i->second.position.x));
        position->SetAttribute("y", std::to_string(i->second.position.y));
        position->SetAttribute("z", std::to_string(i->second.position.z));
        planet->LinkEndChild(position);

        TiXmlElement* velocity = new TiXmlElement("velocity");
        velocity->SetAttribute("x", std::to_string(i->second.velocity.x / velocityfac));
        velocity->SetAttribute("y", std::to_string(i->second.velocity.y / velocityfac));
        velocity->SetAttribute("z", std::to_string(i->second.velocity.z / velocityfac));
        planet->LinkEndChild(velocity);

        root->LinkEndChild(planet);
    }

    doc.LinkEndChild(root);

    if(!doc.SaveFile(filename)){
        errorMsg = doc.ErrorDesc();
        return false;
    }

    return true;
}

void PlanetsUniverse::advance(float time){
    time *= simspeed;
    time /= stepsPerFrame;

    for(int s = 0; s < stepsPerFrame; ++s){
        iterator i = planets.begin();
        while(i != planets.end()){
            if(i->second.mass() <= 0.0f){
                i = planets.erase(i);
            }else{
                iterator o = i; ++o;
                while(o != planets.end()){
                    glm::vec3 direction = o->second.position - i->second.position;
                    float distancesqr = glm::length2(direction);

                    if(distancesqr < glm::pow(i->second.radius() + o->second.radius(), 2.0f)){
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
                        direction *= gravityconst * time * ((o->second.mass() * i->second.mass()) / distancesqr) * glm::fastInverseSqrt(distancesqr);

                        i->second.velocity += direction / i->second.mass();
                        o->second.velocity -= direction / o->second.mass();
                        ++o;
                    }
                }

                i->second.position += i->second.velocity * time;
                i->second.updatePath(pathLength, pathRecordDistance);

                ++i;
            }
        }
    }
}

key_type PlanetsUniverse::addPlanet(const Planet &planet, key_type keyHint){
    uniform_int_distribution<key_type> color_gen(0xFF000001, 0xFFFFFFFF);

    while(planets.count(keyHint) > 0 || (keyHint & RGB_MASK) == 0){
        keyHint = color_gen(generator);
    }

    planets[keyHint] = planet;
    return keyHint;
}

void PlanetsUniverse::generateRandom(const int &count, const float &positionRange, const float &maxVelocity, const float &maxMass){
    uniform_real_distribution<float> position(-positionRange, positionRange);
    uniform_real_distribution<float> velocity(-maxVelocity, maxVelocity);
    uniform_real_distribution<float> mass(min_mass, maxMass);

    for(int i = 0; i < count; ++i){
        addPlanet(Planet(glm::vec3(position(generator), position(generator), position(generator)),
                         glm::vec3(velocity(generator), velocity(generator), velocity(generator)),
                         mass(generator)));
    }
}

/* TODO - This function currently does not account for other planets.
 * Doing so would be very complicated. IDK if it'd even be possible... I'll have to look into it sometime. */
key_type PlanetsUniverse::addOrbital(Planet &around, const float &radius, const float &mass, const glm::mat4& plane){
    /* Calculate the speed based on gravitational force and distance. */
    float speed = sqrt((around.mass() * around.mass() * gravityconst) / ((around.mass() + mass) * radius));

    /* Velocity is the y column of the plane matrix * speed. */
    glm::vec3 velocity = glm::vec3(plane[1]) * speed;

    /* The x column is the relative position of the orbiting planet. */
    key_type planet = addPlanet(Planet(around.position + glm::vec3(plane[0]) * radius, around.velocity + velocity, mass));

    /* Apply force on the planet being orbited in the opposite direction of the resulting planets velocity. */
    around.velocity -= velocity * (mass / around.mass());

    return planet;
}

void PlanetsUniverse::generateRandomOrbital(const int &count, key_type target){
    /* We need a planet to orbit around. */
    if(!isEmpty()){
        /* Ensure we have a valid target planet, if none was provided select at random. */
        if(!isValid(target)){
            uniform_int_distribution<list_type::size_type> random_n(0, size() - 1);
            auto iter = begin();
            std::advance(iter, random_n(generator));
            target = iter->first;
        }

        Planet &around = planets[target];

        uniform_real_distribution<float> angle(-glm::pi<float>(), glm::pi<float>());
        uniform_real_distribution<float> radius(around.radius() * 1.5f, around.radius() * 80.0f);
        uniform_real_distribution<float> mass(min_mass, around.mass() * 0.2f);

        for(int i = 0; i < count; ++i){
            glm::mat4 plane;
            /* This is sort of a cheap way of making a random orbit plane. */
            plane *= glm::rotate(angle(generator), glm::sphericalRand(1.0f));
            plane *= glm::rotate(angle(generator), glm::sphericalRand(1.0f));
            addOrbital(around, radius(generator), mass(generator), plane);
        }
    }
}

void PlanetsUniverse::deleteEscapees(){
    glm::vec3 averagePosition, averageVelocity;
    float totalMass = 0.0f;

    for(const_iterator i = planets.cbegin(); i != planets.cend(); ++i){
        averagePosition += i->second.position * i->second.mass();
        averageVelocity += i->second.velocity * i->second.mass();
        totalMass += i->second.mass();
    }

    averagePosition /= totalMass;
    averageVelocity /= totalMass;

    float limits = 1.0e12f;

    for(iterator i = planets.begin(); i != planets.end();){
        if(glm::distance2(i->second.position, averagePosition) > limits){
            i = planets.erase(i);
        } else{
            ++i;
        }
    }
}

void PlanetsUniverse::centerAll(){
    glm::vec3 averagePosition, averageVelocity;
    float totalMass = 0.0f;

    for(const_iterator i = planets.cbegin(); i != planets.cend(); ++i){
        averagePosition += i->second.position * i->second.mass();
        averageVelocity += i->second.velocity * i->second.mass();
        totalMass += i->second.mass();
    }

    averagePosition /= totalMass;
    averageVelocity /= totalMass;

    float epsilon = glm::epsilon<float>();

    if(!glm::isNull(averagePosition, epsilon) || !glm::isNull(averageVelocity, epsilon)){
        for(iterator i = planets.begin(); i != planets.end(); ++i){
            i->second.position -= averagePosition;
            i->second.velocity -= averageVelocity;
            i->second.path.clear();
        }
    }
}
