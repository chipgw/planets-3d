#include "placinginterface.h"
#include "planetsuniverse.h"
#include "camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

PlacingInterface::PlacingInterface(PlanetsUniverse& u) : universe(u), firingSpeed(universe.velocityfac * 10.0f) {
    planet.velocity.y = universe.velocityfac;
}

bool PlacingInterface::handleMouseMove(const glm::ivec2& pos, const glm::ivec2& delta, const Camera& camera, bool& holdMouse) {
    switch (step) {
    case FreePositionXY: {
        /* Set placing position on the XY plane. */
        Ray ray = camera.getRay(pos);

        planet.position = ray.origin + (ray.direction * ((-ray.origin.z) / ray.direction.z));
        holdMouse = false;
        return true;
    }
    case FreePositionZ:
        /* Set placing Z position. */
        planet.position.z += delta.y * 0.1f;
        holdMouse = true;
        return true;
    case FreeVelocity:
        /* Set placing velocity. */
        rotation *= glm::rotate(delta.x * 1.0e-3f, glm::vec3(1.0f, 0.0f, 0.0f));
        rotation *= glm::rotate(delta.y * 1.0e-3f, glm::vec3(0.0f, 1.0f, 0.0f));
        planet.velocity = glm::vec3(rotation[2]) * glm::length(planet.velocity);
        holdMouse = true;
        return true;
    case OrbitalPlanet:
        if (universe.isSelectedValid()) {
            Ray ray = camera.getRay(pos);

            /* Set the position on an XY plane at the Z of the target planet. */
            planet.position = ray.origin + (ray.direction * ((universe.getSelected().position.z - ray.origin.z) / ray.direction.z));

            /* Get the radius from the position of the orbiting planet relative to the target planet. */
            glm::vec3 relative = planet.position - universe.getSelected().position;
            orbitalRadius = glm::length(relative);

            /* Use the normalized direction between the orbit planet and target to create the rotation matrix. */
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
        if (universe.isSelectedValid()) {
            /* Put mouse delta directly into rotation. */
            rotation *= glm::rotate(delta.x * 1.0e-3f, glm::vec3(1.0f, 0.0f, 0.0f));
            rotation *= glm::rotate(delta.y * 1.0e-3f, glm::vec3(0.0f, 1.0f, 0.0f));

            /* Set the position based on radius and rotation matrix. */
            planet.position = universe.getSelected().position + glm::vec3(rotation[0] * orbitalRadius);
            holdMouse = true;
            return true;
        }
        break;
    }
    return false;
}

bool PlacingInterface::handleMouseClick(const glm::ivec2& pos, const Camera& camera) {
    switch (step) {
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
    case Firing: {
        Ray ray = camera.getRay(pos);

        universe.addPlanet(Planet(ray.origin, ray.direction * firingSpeed, firingMass));
        return true;
    }
    case OrbitalPlanet:
        /* If a planet is selected go to the next step. */
        if (universe.isSelectedValid()) {
            step = OrbitalPlane;
            return true;
        }

        /* If nothing is selected then exit placing mode. */
        orbitalRadius = 0.0f;
        step = NotPlacing;
        break;
    case OrbitalPlane:
        /* No matter what exit placing mode. */
        step = NotPlacing;

        if (universe.isSelectedValid()) {
            universe.addOrbital(universe.getSelected(), orbitalRadius, planet.mass(), rotation);

            orbitalRadius = 0.0f;
            return true;
        }
        orbitalRadius = 0.0f;
        break;
    }
    return false;
}

bool PlacingInterface::handleMouseWheel(float delta) {
    switch (step) {
    case FreePositionXY:
    case FreePositionZ:
    case OrbitalPlanet:
    case OrbitalPlane:
        planet.setMass(glm::clamp(planet.mass() + delta * planet.mass(), universe.min_mass, universe.max_mass));
        return true;
    case FreeVelocity:
        planet.velocity = glm::vec3(rotation[2]) * glm::max(0.0f, glm::length(planet.velocity) + delta * universe.velocityfac);
        return true;
    }
    return false;
}

bool PlacingInterface::handleAnalogStick(const glm::vec2& pos, const bool& modifier, Camera& camera) {
    if (modifier) {
        float y = pos.y * 10.0f;
        switch (step) {
        case FreePositionXY:
        case FreePositionZ:
        case OrbitalPlanet:
        case OrbitalPlane:
            planet.setMass(glm::clamp(planet.mass() - y * planet.mass(), universe.min_mass, universe.max_mass));
            return true;
        case FreeVelocity:
            planet.velocity = glm::vec3(rotation[2]) * glm::max(0.0f, glm::length(planet.velocity) - y * universe.velocityfac);
            return true;
        }
    }
    switch (step) {
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
        if (universe.isSelectedValid()) {
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
        if (universe.isSelectedValid()) {
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

void PlacingInterface::enableFiringMode(bool enable) {
    if (enable) {
        step = Firing;
        universe.resetSelected();
    } else if (step == Firing) {
        step = NotPlacing;
    }
}

void PlacingInterface::beginInteractiveCreation() {
    step = FreePositionXY; universe.resetSelected();
}

void PlacingInterface::beginOrbitalCreation() {
    if (universe.isSelectedValid()) step = OrbitalPlanet;
}

glm::mat4 PlacingInterface::getOrbitalCircleMat() {
    /* This is how large the orbit of the planet being placed will be. */
    float radius = orbitalRadius / (1 + (planet.mass() / universe.getSelected().mass()));

    /* Start at the placing planet location and move -1 (* the radius value) on x to locate the center. */
    glm::mat4 matrix = glm::translate(planet.position);
    matrix = glm::scale(matrix, glm::vec3(radius));
    matrix *= rotation;
    matrix = glm::translate(matrix, glm::vec3(-1.0f, 0.0f, 0.0f));

    return matrix;
}

glm::mat4 PlacingInterface::getOrbitedCircleMat() {
    /* This is how much the planet being orbited around will be displaced by the new planet. */
    float radius = orbitalRadius / (1 + (universe.getSelected().mass() / planet.mass()));

    /* Start at the selected planet location and move 1 (* the radius value) on x to locate the center. */
    glm::mat4 matrix = glm::translate(universe.getSelected().position);
    matrix = glm::scale(matrix, glm::vec3(radius));
    matrix *= rotation;
    matrix = glm::translate(matrix, glm::vec3( 1.0f, 0.0f, 0.0f));

    return matrix;
}
