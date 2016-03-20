#include "camera.h"
#include "planet.h"
#include "planetsuniverse.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

Camera::Camera(PlanetsUniverse& u) : universe(u) {
    reset();
}

void Camera::bound() {
    distance = glm::clamp(distance, 10.0f, 1.0e4f);
    xrotation = glm::clamp(xrotation, -glm::half_pi<float>(), glm::half_pi<float>());
    zrotation = glm::mod(zrotation, glm::pi<float>() * 2.0f);
}

void Camera::reset() {
    /* Here are the Camera default values. */
    distance = 100.0f;
    xrotation = glm::quarter_pi<float>();
    zrotation = 0.0f;
    clearFollow();
    position = glm::vec3();
}

void Camera::resizeViewport(const float& width, const float& height) {
    viewport = glm::vec4(0.0f, 0.0f, width, height);

    projection = glm::perspective(glm::quarter_pi<float>(), width / height, 0.1f, 1.0e6f);
}

const glm::mat4& Camera::setup() {
    /* If universe is empty following is useless. */
    if (!universe.isEmpty()) {
        switch (followingState) {
        case Single:
            if (universe.isValid(universe.following))
                position = universe[universe.following].position;
            else
                /* If the following target is invalid, reset following state. */
                followingState = FollowNone;
            break;
        case PlainAverage:
            position = glm::vec3();
            for (const auto& i : universe)
                position += i.second.position;

            position /= universe.size();
            break;
        case WeightedAverage:
            position = glm::vec3();
            float totalMass = 0.0f;

            for (const auto& i : universe) {
                position += i.second.position * i.second.mass();
                totalMass += i.second.mass();
            }
            position /= totalMass;
            break;
        }
    }

    /* First move the camera the distance amount back on Z. */
    camera = glm::translate(projection, glm::vec3(0.0f, 0.0f, -distance));
    /* Rotate camera around x axis, subtract pi/2 to make the rotation origin be the camera on the y axis. */
    camera = glm::rotate(camera, xrotation - glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));
    /* Rotate the camera on the z axis. */
    camera = glm::rotate(camera, zrotation, glm::vec3(0.0f, 0.0f, 1.0f));
    /* Finally, translate to the position. */
    camera = glm::translate(camera, -position);

    /* Return the camera matrix. */
    return camera;
}

Ray Camera::getRay(const glm::ivec2& pos, float startDepth, float endDepth) const {
    Ray ray;

    /* There is no model matrix, just use identity. */
    glm::mat4 model;
    /* viewport.z = width and viewport.w = height. */
    glm::vec3 windowCoord(pos.x, viewport.w - pos.y, startDepth);

    ray.origin = glm::unProject(windowCoord, model, camera, viewport);

    windowCoord.z = endDepth;

    /* The normalized difference between the unprojected mouse coordinates
     * with a z of >0 and a z of 0 gives us a vector pointing straight out from the camera. */
    ray.direction = glm::normalize(glm::unProject(windowCoord, model, camera, viewport) - ray.origin);

    return ray;
}

key_type Camera::selectUnder(const glm::ivec2& pos, float scale) {
    universe.resetSelected();
    float nearest = std::numeric_limits<float>::max();

    scale *= scale;

    Ray ray = getRay(pos);

    /* Go through each planet and see if the ray intersects it. */
    for (const auto& i : universe) {
        /* Find the directional vector from the ray origin to the planet. */
        glm::vec3 difference = i.second.position - ray.origin;

        float dot = glm::dot(difference, ray.direction);

        /* Getting distance^2 works just fine for us, no need to sqrt. */
        float distance = glm::length2(difference);

        /* distance^2 - dot^2 is the closest the ray gets to the planet's center point.
         * Comparing to the planet radius tells whether or not it intersects. */
        if (distance < nearest && (distance - dot * dot) <= (i.second.radius() * i.second.radius() * scale)) {
            universe.selected = i.first;
            nearest = distance;
        }
    }

	return universe.selected;
}

void Camera::clearFollow() {
    universe.following = 0;
    followingState = FollowNone;
}

void Camera::followNext() {
    if (!universe.isEmpty()) {
        followingState = Single;
        PlanetsUniverse::const_iterator current = universe.find(universe.following);

        if (current == universe.cend())
            /* If the planet was not found, start at the beginning. */
            current = universe.cbegin();
        else if (++current == universe.cend())
            /* If the planet was the last planet in the list, start at the beginning. */
            current = universe.cbegin();

        /* Get the key back from the iterator. */
        universe.following = current->first;
    }
}

void Camera::followPrevious() {
    if (!universe.isEmpty()) {
        followingState = Single;
        PlanetsUniverse::const_iterator current = universe.find(universe.following);

        if (current == universe.cend()) {
            /* If the planet was not found, start at the beginning. */
            current = universe.cbegin();
        } else {
            /* If the planet was the first planet in the list, start at the end. */
            if (current == universe.cbegin())
                current = universe.cend();
            --current;
        }

        /* Get the key back from the iterator. */
        universe.following = current->first;
    }
}

void Camera::followSelection() {
    universe.following = universe.selected;
    followingState = Single;
}

glm::ivec2 Camera::getCenterScreen() const {
     return glm::ivec2(viewport.z / 2, viewport.w / 2);
}
