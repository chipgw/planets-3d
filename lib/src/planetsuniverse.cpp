#include "planetsuniverse.h"
#include "planet.h"
#include <chrono>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/vector_query.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>
#include <cstdio>

#ifndef EMSCRIPTEN
#include <tinyxml.h>
#endif

using std::uniform_int_distribution;
using std::uniform_real_distribution;

PlanetsUniverse::PlanetsUniverse() : generator(std::chrono::system_clock::now().time_since_epoch().count()) { }

/* Emscripten does IO from javascript. */
#ifndef EMSCRIPTEN

int PlanetsUniverse::load(const std::string& filename, bool clear) {
    TiXmlDocument doc(filename);

    if (!doc.LoadFile())
        throw std::runtime_error("Unable to load file \"" + filename + "\"!\n" + doc.ErrorDesc());

    TiXmlElement* root = doc.FirstChildElement("planets-3d-universe");
    if (root == nullptr)
        throw std::runtime_error("\"" + filename + "\" is not a valid universe file!");

    if (clear)
        deleteAll();

    /* Keep track of how many planets were loaded. */
    int loaded = 0;

    for (TiXmlElement* element = root->FirstChildElement(); element != nullptr; element = element->NextSiblingElement()) {
        if (element->ValueStr() == "planet") {
            Planet planet;
            planet.setMass(std::stof(element->Attribute("mass")));

            for (TiXmlElement* sub = element->FirstChildElement(); sub != nullptr; sub = sub->NextSiblingElement()) {
                if (sub->ValueStr() == "position")
                    planet.position = glm::vec3(std::stof(sub->Attribute("x")),
                                                std::stof(sub->Attribute("y")),
                                                std::stof(sub->Attribute("z")));
                else if (sub->ValueStr() == "velocity")
                    /* Velocity is saved with velocity factor. */
                    planet.velocity = glm::vec3(std::stof(sub->Attribute("x")),
                                                std::stof(sub->Attribute("y")),
                                                std::stof(sub->Attribute("z"))) * velocityfac;
            }
            addPlanet(planet);
            ++loaded;
        }
    }

    return loaded;
}

void PlanetsUniverse::save(const std::string& filename) {
    TiXmlDocument doc;

    doc.LinkEndChild(new TiXmlDeclaration("1.0", "", ""));

    TiXmlElement* root = new TiXmlElement("planets-3d-universe");

    for (const Planet& planet : planets) {
        TiXmlElement* element = new TiXmlElement("planet");
        element->SetAttribute("mass", std::to_string(planet.mass()));

        TiXmlElement* position = new TiXmlElement("position");
        position->SetAttribute("x", std::to_string(planet.position.x));
        position->SetAttribute("y", std::to_string(planet.position.y));
        position->SetAttribute("z", std::to_string(planet.position.z));
        element->LinkEndChild(position);

        TiXmlElement* velocity = new TiXmlElement("velocity");
        /* Velocity is saved with velocity factor. */
        velocity->SetAttribute("x", std::to_string(planet.velocity.x / velocityfac));
        velocity->SetAttribute("y", std::to_string(planet.velocity.y / velocityfac));
        velocity->SetAttribute("z", std::to_string(planet.velocity.z / velocityfac));
        element->LinkEndChild(velocity);

        root->LinkEndChild(element);
    }

    doc.LinkEndChild(root);

    if (!doc.SaveFile(filename))
        /* If TinyXML had an error, throw it. */
        throw std::runtime_error(doc.ErrorDesc());
}

#endif

/* Basically the Quake method, tweaked for as much performance as I could get. */
static float fastInverseSqrt(float x) {
    float halfx = x * 0.5f;
    int32_t& i = reinterpret_cast<int32_t&>(x);
    i = 0x5f3759df - (i >> 1);
    return x*(1.5f - halfx*x*x);
}

void PlanetsUniverse::advance(float time) {
    /* Factor the simulation speed and number of steps into the time value. */
    time *= simspeed / stepsPerFrame;
    float gconsttime = gravityconst * time;
    iterator e = planets.end();

    for (int s = 0; s < stepsPerFrame; ++s) {
        for (iterator i = planets.begin(); i != e;) {
            /* We only have to run this for planets after the current one,
             * because all the planets before this have already been calculated with this one. */
            for (iterator o = i + 1; o != e;) {
                glm::vec3 direction = o->position - i->position;
                /* Don't use glm::length2 because it involves a conversion and extra multiply & add operations for a forth component. */
                float force = direction.x * direction.x + direction.y * direction.y + direction.z * direction.z;

                /* Planets are close enough to merge. */
                if (force < (i->radius() + o->radius()) * (i->radius() + o->radius())) {
                    /* Set the position and velocity to the wieghted average between the planets. */
                    i->position = o->position * o->mass() + i->position * i->mass();
                    i->velocity = o->velocity * o->mass() + i->velocity * i->mass();

                    /* Add the masses together. */
                    i->setMass(i->mass() + o->mass());

                    /* Finish the weighted average calculation. */
                    i->position /= i->mass();
                    i->velocity /= i->mass();

                    /* The path would be invalid after this. */
                    i->path.clear();

                    /* This function checks selected and following to make sure they remain valid. */
                    remove(o - begin(), i - begin());
                    e = planets.end();
                } else {
                    /* The gravity math to calculate the force between the planets. */
                    force = gconsttime / force * fastInverseSqrt(force);

                    /* Apply the force to the velocity of both planets. */
                    i->velocity += force * o->mass() * direction;
                    o->velocity -= force * i->mass() * direction;

                    /* Keep going. (Not in for loop because of the possibility of erase() getting called.) */
                    ++o;
                }
            }

            /* Apply the velocity to the position of the planet and update the path. */
            i->position += i->velocity * time;
            i->updatePath(pathLength, pathRecordDistance);

            /* Onward! (Same situation as with ++o above.) */
            ++i;
        }
    }
}

void PlanetsUniverse::remove(const key_type key, const key_type replacement) {
    if (!isValid(key))
        return;

    /* If the one we're deleting happens to be selected, select the remaining planet. */
    if (key == selected)
        selected = replacement;
    /* If the selected planet is after the one being deleted, it will shift down one location in the list. */
    else if (key < selected)
        --selected;

    /* If the planet being followed happens to be the one being deleted, follow the remaining planet. */
    if (key == following)
        following = replacement;
    /* If the planet being followed is after the one being deleted, it will shift down one location in the list. */
    else if (key < following)
        --following;

    planets.erase(begin() + key);
}

void PlanetsUniverse::generateRandom(const size_t& count, const float& positionRange, const float& maxVelocity, const float& maxMass) {
    uniform_real_distribution<float> position(-positionRange, positionRange);
    uniform_real_distribution<float> velocity(-maxVelocity, maxVelocity);
    uniform_real_distribution<float> mass(min_mass, maxMass);

    for (int i = 0; i < count; ++i)
        addPlanet(Planet(glm::vec3(position(generator), position(generator), position(generator)),
                         glm::vec3(velocity(generator), velocity(generator), velocity(generator)),
                         mass(generator)));
}

/* TODO - This function currently does not account for other planets.
 * Doing so would be very complicated. IDK if it'd even be possible... I'll have to look into it sometime. */
key_type PlanetsUniverse::addOrbital(Planet& around, const float& radius, const float& mass, const glm::mat4& plane) {
    /* Calculate the speed based on gravitational force and distance. */
    float speed = sqrt((around.mass() * around.mass() * gravityconst) / ((around.mass() + mass) * radius));

    /* Velocity is the y column of the plane matrix * speed. */
    glm::vec3 velocity = glm::vec3(plane[1]) * speed;

    /* The x column is the relative position of the orbiting planet. */
    Planet planet(around.position + glm::vec3(plane[0]) * radius, around.velocity + velocity, mass);

    /* Apply force on the planet being orbited in the opposite direction of the resulting planets velocity. */
    around.velocity -= velocity * (mass / around.mass());

    return addPlanet(planet);
}

void PlanetsUniverse::generateRandomOrbital(const size_t& count, key_type target) {
    /* We need a planet to orbit around. */
    if (!isEmpty()) {
        /* Ensure we have a valid target planet, if none was provided select at random. */
        if (!isValid(target))
            target = getRandomPlanet();

        Planet &around = planets[target];

        uniform_real_distribution<float> angle(-glm::pi<float>(), glm::pi<float>());
        uniform_real_distribution<float> radius(around.radius() * 1.5f, around.radius() * 80.0f);
        uniform_real_distribution<float> mass(min_mass, around.mass() * 0.2f);

        for (int i = 0; i < count; ++i) {
            glm::mat4 plane;
            /* This is sort of a cheap way of making a random orbit plane. */
            plane *= glm::rotate(angle(generator), glm::sphericalRand(1.0f));
            plane *= glm::rotate(angle(generator), glm::sphericalRand(1.0f));
            addOrbital(around, radius(generator), mass(generator), plane);
        }
    }
}

void PlanetsUniverse::deleteEscapees() {
    /* We delete anything too far from the weighted average position. */
    glm::vec3 averagePosition;
    float totalMass = 0.0f;

    for (const auto& planet : planets) {
        averagePosition += planet.position * planet.mass();
        totalMass += planet.mass();
    }

    averagePosition /= totalMass;

    /* The squared distance from the center outside of which we delete things. */
    const float limits2 = 1.0e12f;

    for (int i = 0; i < size();) {
        if (glm::distance2(planets[i].position, averagePosition) > limits2)
            remove(i);
        else
            ++i;
    }
}

key_type PlanetsUniverse::getRandomPlanet() {
    if (isEmpty()) return 0;

    uniform_int_distribution<list_type::size_type> random_n(0, size() - 1);

    return random_n(generator);
}

void PlanetsUniverse::centerAll() {
    /* We need the weighted average position and velocity to center. */
    glm::vec3 averagePosition, averageVelocity;
    float totalMass = 0.0f;

    for (const auto& planet : planets) {
        averagePosition += planet.position * planet.mass();
        averageVelocity += planet.velocity * planet.mass();
        totalMass += planet.mass();
    }

    averagePosition /= totalMass;
    averageVelocity /= totalMass;

    const float epsilon = glm::epsilon<float>();

    /* Don't bother centering if we're already reasonably centered. */
    if (!glm::isNull(averagePosition, epsilon) || !glm::isNull(averageVelocity, epsilon)) {
        for (auto& planet : planets) {
            planet.position -= averagePosition;
            planet.velocity -= averageVelocity;
            planet.path.clear();
        }
    }
}
