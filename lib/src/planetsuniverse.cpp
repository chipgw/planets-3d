#include "include/planetsuniverse.h"
#include <chrono>
#include <tinyxml.h>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/vector_query.hpp>
#include <glm/gtx/fast_square_root.hpp>
#include <cstdio>

#define RGB_MASK 0x00ffffff

using std::uniform_int_distribution;
using std::uniform_real_distribution;

PlanetsUniverse::PlanetsUniverse() : selected(0), simspeed(1.0f), stepsPerFrame(20), generator(unsigned int(std::chrono::system_clock::now().time_since_epoch().count())) {}

bool PlanetsUniverse::load(const std::string &filename, bool clear){
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
                    planet.position = glm::vec3(std::stof(sub->Attribute("x")), std::stof(sub->Attribute("y")), std::stof(sub->Attribute("z")));
                }else if(sub->ValueStr() == "velocity"){
                    planet.velocity = glm::vec3(std::stof(sub->Attribute("x")), std::stof(sub->Attribute("y")), std::stof(sub->Attribute("z"))) * velocityfac;
                }
            }
            addPlanet(planet, color);
        }
    }

    return true;
}

bool PlanetsUniverse::save(const std::string &filename){
    TiXmlDocument doc;

    doc.LinkEndChild(new TiXmlDeclaration("1.0", "", ""));

    TiXmlElement* root = new TiXmlElement("planets-3d-universe");

    for(const_iterator i = planets.cbegin(); i != planets.cend(); ++i){
        TiXmlElement* planet = new TiXmlElement("planet");
        planet->SetAttribute("mass", std::to_string(i->second.mass()));

        // Puts uint32_t key into hexadecimal format #RRGGBB (No need to save alpha, as that's always 0xff)
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
        for(iterator i = planets.begin(); i != planets.end();){
            if(i->second.mass() <= 0.0f){
                i = planets.erase(i);
            }else{
                iterator o = i; ++o;
                for(;o != planets.end();){
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
                i->second.updatePath();

                ++i;
            }
        }
    }
}

PlanetsUniverse::key_type PlanetsUniverse::addPlanet(const Planet &planet, key_type colorhint){
    uniform_int_distribution<key_type> color_gen(0xFF000001, 0xFFFFFFFF);

    while(planets.count(colorhint) > 0 || (colorhint & RGB_MASK) == 0){
        colorhint = color_gen(generator);
    }

    planets[colorhint] = planet;
    return colorhint;
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

void PlanetsUniverse::deleteAll(){
    planets.clear();
    resetSelected();
}

void PlanetsUniverse::deleteEscapees(){
    glm::vec3 averagePosition, averageVelocity;
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
        if(glm::distance2(i->second.position, averagePosition) > limits){
            i = planets.erase(i);
        } else{
            ++i;
        }
    }
}

void PlanetsUniverse::deleteSelected(){
    if(isSelectedValid()){
        planets.erase(selected);
    }
}

void PlanetsUniverse::centerAll(){
    glm::vec3 averagePosition, averageVelocity;
    float totalmass = 0.0f;

    for(const_iterator i = planets.cbegin(); i != planets.cend(); ++i){
        averagePosition += i->second.position * i->second.mass();
        averageVelocity += i->second.velocity * i->second.mass();
        totalmass += i->second.mass();
    }

    averagePosition /= totalmass;
    averageVelocity /= totalmass;

    float epsilon = std::numeric_limits<float>::epsilon();

    if(!glm::isNull(averagePosition, epsilon) || !glm::isNull(averageVelocity, epsilon)){
        for(iterator i = planets.begin(); i != planets.end(); ++i){
            i->second.position -= averagePosition;
            i->second.velocity -= averageVelocity;
            i->second.path.clear();
        }
    }
}

const float PlanetsUniverse::gravityconst = 6.67e-11f;

const float PlanetsUniverse::velocityfac = 1.0e-5f;

const float PlanetsUniverse::min_mass = 1.0f;
const float PlanetsUniverse::max_mass = 1.0e9f;
