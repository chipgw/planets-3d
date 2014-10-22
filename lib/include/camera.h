#ifndef CAMERA_H
#define CAMERA_H

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <planetsuniverse.h>

struct Ray {
    glm::vec3 origin, direction;
};

class Camera {
    PlanetsUniverse &universe;

public:
    enum FollowingState{
        FollowNone,
        Single,
        PlainAverage,
        WeightedAverage
    };

    FollowingState followingState;
    PlanetsUniverse::key_type following;

    glm::vec3 position;
    float distance;
    float xrotation;
    float zrotation;

    glm::mat4 projection;
    glm::mat4 camera;

    Camera(PlanetsUniverse &universe);

    void bound();
    void reset();
    void resizeViewport(const float &width, const float &height);
    const glm::mat4 &setup();

    Ray getRay(const glm::ivec2 &pos, const int &windowW, const int &windowH, bool normalize, float startDepth = 0.9f, float endDepth = 0.0f) const;
    PlanetsUniverse::key_type selectUnder(const glm::ivec2 &pos, const int &windowW, const int &windowH);

    void followPrevious();
    void followNext();
    void followSelection() { following = universe.selected; followingState = Single; }
    void clearFollow() { following = 0; followingState = FollowNone; }
    void followPlainAverage() { followingState = PlainAverage; }
    void followWeightedAverage() { followingState = WeightedAverage; }
};

#endif // CAMERA_H
