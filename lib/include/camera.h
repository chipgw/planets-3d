#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "types.h"

/* Simple way to return the origin and direction of a ray. */
struct Ray {
    glm::vec3 origin, direction;
};

class Camera {
    PlanetsUniverse& universe;

    /* [0, 0, Width, Height] (in pixels). Used for unproject and anything else that wants width or height of the viewport. */
    glm::vec4 viewport;

public:
    enum FollowingState{
        FollowNone,
        Single,
        PlainAverage,
        WeightedAverage
    };

    /* What are we following? */
    FollowingState followingState;

    /* Values to transform the camera by. Rotation values are in radians. */
    glm::vec3 position;
    float distance;
    float xrotation;
    float zrotation;

    /* The perspective camera matrix, before transformations are applied. */
    glm::mat4 projection;
    /* The final camera matrix with all transformations applied, pass this to OpenGL. */
    glm::mat4 camera;

    EXPORT Camera(PlanetsUniverse& universe);

    /* Limit the rotation & distance of the camera. */
    EXPORT void bound();
    /* Reset to the default camera position & rotation, also clearing the follow state. */
    EXPORT void reset();

    /* Call this from the interface code when the viewport size changes. */
    EXPORT void resizeViewport(const float& width, const float& height);

    /* Update the camera matrix from all the other variables. */
    EXPORT const glm::mat4& setup();

    /* Get a ray coming from the camera at pos viewport coordinates. */
    EXPORT Ray getRay(const glm::ivec2& pos, float startDepth = 0.0f, float endDepth = 0.9f) const;
    /* Select the planet under the viewport coordinates supplied. */
    EXPORT key_type selectUnder(const glm::ivec2& pos);

    /* Following state changing functions. */
    EXPORT void followPrevious();
    EXPORT void followNext();
    EXPORT void followSelection();
    EXPORT void clearFollow();
    inline void followPlainAverage() { followingState = PlainAverage; }
    inline void followWeightedAverage() { followingState = WeightedAverage; }

    EXPORT glm::ivec2 getCenterScreen() const;
};
