#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <planetsuniverse.h>

/* Simple way to return the origin and direction of a ray. */
struct Ray {
    glm::vec3 origin, direction;
};

class Camera {
    PlanetsUniverse &universe;

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
    PlanetsUniverse::key_type following;

    /* Values to transform the camera by. Rotation values are in radians. */
    glm::vec3 position;
    float distance;
    float xrotation;
    float zrotation;

    /* The perspective camera matrix, before transformations are applied. */
    glm::mat4 projection;
    /* The final camera matrix with all transformations applied, pass this to OpenGL. */
    glm::mat4 camera;

    Camera(PlanetsUniverse &universe);

    /* Limit the rotation & distance of the camera. */
    void bound();
    /* Reset to the default camera position & rotation, also clearing the follow state. */
    void reset();

    /* Call this from the interface code when the viewport size changes. */
    void resizeViewport(const float &width, const float &height);

    /* Update the camera matrix from all the other variables. */
    const glm::mat4 &setup();

    /* Get a ray coming from the camera at pos viewport coordinates. */
    Ray getRay(const glm::ivec2 &pos, float startDepth = 0.9f, float endDepth = 0.0f) const;
    /* Select the planet under the viewport coordinates supplied. */
    PlanetsUniverse::key_type selectUnder(const glm::ivec2 &pos);

    /* Following state changing functions. */
    void followPrevious();
    void followNext();
    void followSelection() { following = universe.selected; followingState = Single; }
    void clearFollow() { following = 0; followingState = FollowNone; }
    void followPlainAverage() { followingState = PlainAverage; }
    void followWeightedAverage() { followingState = WeightedAverage; }
};
