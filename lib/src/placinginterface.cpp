#include "include/placinginterface.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

PlacingInterface::PlacingInterface(PlanetsUniverse &u) : universe(u), firingSpeed(PlanetsUniverse::velocityfac * 10.0f), firingMass(25.0f) {
    planet.velocity.y = PlanetsUniverse::velocityfac;
}

bool PlacingInterface::handleMouseMove(const glm::ivec2 &pos, const glm::ivec2 &delta, const int &windowW, const int &windowH, const Camera &camera, bool &holdMouse){
    switch(step){
    case FreePositionXY:{
        // set placing position on the XY plane
        Ray ray = camera.getRay(pos, windowW, windowH, false);

        planet.position = ray.origin + (ray.direction * ((-ray.origin.z) / ray.direction.z));
        holdMouse = false;
        return true;
    }
    case FreePositionZ:
        // set placing Z position
        planet.position.z += delta.y * 0.1f;
        holdMouse = true;
        return true;
    case FreeVelocity:
        // set placing velocity
        rotation *= glm::rotate(delta.x * 0.05f, glm::vec3(1.0f, 0.0f, 0.0f));
        rotation *= glm::rotate(delta.y * 0.05f, glm::vec3(0.0f, 1.0f, 0.0f));
        planet.velocity = glm::vec3(rotation[2]) * glm::length(planet.velocity);
        holdMouse = true;
        return true;
    case OrbitalPlanet:
        if(universe.isSelectedValid()){
            Ray ray = camera.getRay(pos, windowW, windowH, false);

            planet.position = ray.origin + (ray.direction * ((universe.getSelected().position.z - ray.origin.z) / ray.direction.z));
            glm::vec3 relative = planet.position - universe.getSelected().position;
            orbitalRadius = glm::length(relative);
            relative /= orbitalRadius;
            rotation = glm::mat4(glm::vec4(relative, 0.0f),
                                        glm::vec4(relative.y, -relative.x, 0.0f, 0.0f),
                                        glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
                                        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            holdMouse = false;
            return true;
        }
        break;
    case OrbitalPlane:
        if(universe.isSelectedValid()){
            rotation *= glm::rotate(delta.x * 0.05f, glm::vec3(1.0f, 0.0f, 0.0f));
            rotation *= glm::rotate(delta.y * 0.05f, glm::vec3(0.0f, 1.0f, 0.0f));
            planet.position = universe.getSelected().position + glm::vec3(rotation[0] * orbitalRadius);
            holdMouse = true;
            return true;
        }
        break;
    }
    return false;
}

bool PlacingInterface::handleMouseClick(const glm::ivec2 &pos, const int &windowW, const int &windowH, const Camera &camera){
    switch(step){
    case FreePositionXY:
        step = FreePositionZ;
        return true;
    case FreePositionZ:
        step = FreeVelocity;
        return true;
    case FreeVelocity:
        step = NotPlacing;
        planet.velocity = glm::vec3(rotation[2]) * glm::length(planet.velocity);
        universe.selected = universe.addPlanet(planet);
        return true;
    case Firing:{
        Ray ray = camera.getRay(pos, windowW, windowH, true);

        universe.addPlanet(Planet(ray.origin, ray.direction * firingSpeed, firingMass));
        return true;
    }
    case OrbitalPlanet:
        if(universe.isSelectedValid()){
            step = OrbitalPlane;
            return true;
        }
        orbitalRadius = 0.0f;
        step = NotPlacing;
        break;
    case OrbitalPlane:
        if(universe.isSelectedValid()){
            Planet &selected = universe.getSelected();
            float speed = sqrt((selected.mass() * selected.mass() * PlanetsUniverse::gravityconst) / ((selected.mass() + planet.mass()) * orbitalRadius));
            glm::vec3 velocity = glm::vec3(rotation[1]) * speed;
            universe.selected = universe.addPlanet(Planet(planet.position, selected.velocity + velocity, planet.mass()));
            selected.velocity -= velocity * (planet.mass() / selected.mass());

            orbitalRadius = 0.0f;
            step = NotPlacing;
            return true;
        }
        orbitalRadius = 0.0f;
        step = NotPlacing;
        break;
    }
    return false;
}

bool PlacingInterface::handleMouseWheel(float delta){
    switch(step){
    case FreePositionXY:
    case FreePositionZ:
    case OrbitalPlanet:
    case OrbitalPlane:
        planet.setMass(glm::clamp(planet.mass() + delta * planet.mass() * 1.0e-3f, PlanetsUniverse::min_mass, PlanetsUniverse::max_mass));
        return true;
    case FreeVelocity:
        planet.velocity = glm::vec3(rotation[2]) * glm::max(0.0f, glm::length(planet.velocity) + delta * PlanetsUniverse::velocityfac * 1.0e-3f);
        return true;
    }
    return false;
}

void PlacingInterface::enableFiringMode(bool enable){
    if(enable){
        step = Firing;
        universe.resetSelected();
    }else if(step == Firing){
        step = NotPlacing;
    }
}
