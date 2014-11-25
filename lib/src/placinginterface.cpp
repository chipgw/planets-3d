#include "include/placinginterface.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

PlacingInterface::PlacingInterface(PlanetsUniverse &u) : universe(u), firingSpeed(PlanetsUniverse::velocityfac * 10.0f), firingMass(25.0f), step(NotPlacing) {
    planet.velocity.y = PlanetsUniverse::velocityfac;
}

bool PlacingInterface::handleMouseMove(const glm::ivec2 &pos, const glm::ivec2 &delta, const Camera &camera, bool &holdMouse){
    switch(step){
    case FreePositionXY:{
        // set placing position on the XY plane
        Ray ray = camera.getRay(pos);

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
        rotation *= glm::rotate(delta.x * 1.0e-3f, glm::vec3(1.0f, 0.0f, 0.0f));
        rotation *= glm::rotate(delta.y * 1.0e-3f, glm::vec3(0.0f, 1.0f, 0.0f));
        planet.velocity = glm::vec3(rotation[2]) * glm::length(planet.velocity);
        holdMouse = true;
        return true;
    case OrbitalPlanet:
        if(universe.isSelectedValid()){
            Ray ray = camera.getRay(pos);

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
            rotation *= glm::rotate(delta.x * 1.0e-3f, glm::vec3(1.0f, 0.0f, 0.0f));
            rotation *= glm::rotate(delta.y * 1.0e-3f, glm::vec3(0.0f, 1.0f, 0.0f));
            planet.position = universe.getSelected().position + glm::vec3(rotation[0] * orbitalRadius);
            holdMouse = true;
            return true;
        }
        break;
    }
    return false;
}

bool PlacingInterface::handleMouseClick(const glm::ivec2 &pos, const Camera &camera){
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
        Ray ray = camera.getRay(pos);

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
        planet.setMass(glm::clamp(planet.mass() + delta * planet.mass(), PlanetsUniverse::min_mass, PlanetsUniverse::max_mass));
        return true;
    case FreeVelocity:
        planet.velocity = glm::vec3(rotation[2]) * glm::max(0.0f, glm::length(planet.velocity) + delta * PlanetsUniverse::velocityfac);
        return true;
    }
    return false;
}

bool PlacingInterface::handleAnalogStick(const glm::vec2 &pos, const bool& modifier, Camera &camera){
    if(modifier){
        float y = pos.y * 10.0f;
        switch(step){
        case FreePositionXY:
        case FreePositionZ:
        case OrbitalPlanet:
        case OrbitalPlane:
            planet.setMass(glm::clamp(planet.mass() - y * planet.mass(), PlanetsUniverse::min_mass, PlanetsUniverse::max_mass));
            return true;
        case FreeVelocity:
            planet.velocity = glm::vec3(rotation[2]) * glm::max(0.0f, glm::length(planet.velocity) - y * PlanetsUniverse::velocityfac);
            return true;
        }
    }
    switch(step){
    case FreePositionXY:
        /* Set placing position on XY plane. */
        planet.position += glm::rotateZ(glm::vec3(pos.x, -pos.y, 0.0f) * camera.distance, -camera.zrotation);
        camera.position = planet.position;
        return true;
    case FreePositionZ:
        /* Set placing position on Z axis. */
        planet.position.z -= pos.y * camera.distance;
        camera.position = planet.position;
        return true;
    case FreeVelocity:
        /* Rotate initial velocity. */
        rotation *= glm::rotate(pos.x, glm::vec3(1.0f, 0.0f, 0.0f));
        rotation *= glm::rotate(pos.y, glm::vec3(0.0f, 1.0f, 0.0f));
        planet.velocity = glm::vec3(rotation[2]) * glm::length(planet.velocity);
        return true;
    case OrbitalPlanet:
        if(universe.isSelectedValid()){
            /* In this step the planet is locked at the same z coord as the one it's orbiting. */
            planet.position.z = universe.getSelected().position.z;

            /* Set placing position on XY plane. */
            planet.position += glm::rotateZ(glm::vec3(pos.x, -pos.y, 0.0f) * camera.distance, -camera.zrotation);
            camera.position = planet.position;

            /* Calculate the radius and rotation matrix from the planet's position */
            glm::vec3 relative = planet.position - universe.getSelected().position;
            orbitalRadius = glm::length(relative);
            relative /= orbitalRadius;
            rotation = glm::mat4(glm::vec4(relative, 0.0f),
                                 glm::vec4(relative.y, -relative.x, 0.0f, 0.0f),
                                 glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
                                 glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            return true;
        }
        break;
    case OrbitalPlane:
        if(universe.isSelectedValid()){
            /* TODO - this doesn't work as well as it did with the mouse, need to find a better method */
            rotation *= glm::rotate(pos.x, glm::vec3(1.0f, 0.0f, 0.0f));
            rotation *= glm::rotate(pos.y, glm::vec3(0.0f, 1.0f, 0.0f));
            planet.position = universe.getSelected().position + glm::vec3(rotation[0] * orbitalRadius);

            /* Center the camera on the selected planet for this step. */
            camera.position = universe.getSelected().position;
            return true;
        }
        break;
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
