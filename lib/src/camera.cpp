#include "camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

Camera::Camera(PlanetsUniverse &u) : universe(u) {
    reset();
}

void Camera::bound(){
    distance = glm::clamp(distance, 10.0f, 1.0e4f);
    xrotation = glm::clamp(xrotation, -90.0f, 90.0f);
    zrotation = glm::mod(zrotation, 360.0f);
}

void Camera::reset(){
    distance = 100.0f;
    xrotation = 45.0f;
    zrotation = 0.0f;
}

void Camera::resizeViewport(const float &width, const float &height){
    projection = glm::perspective(45.0f, width / height, 0.1f, 1.0e6f);
}

const glm::mat4 &Camera::setup(){
    glm::vec3 finalPos;

    switch(followingState){
    case WeightedAverage:
        if(universe.size() != 0){
            float totalmass = 0.0f;

            for(const auto& i : universe){
                finalPos += i.second.position * i.second.mass();
                totalmass += i.second.mass();
            }
            finalPos /= totalmass;
        }
        break;
    case PlainAverage:
        if(universe.size() != 0){
            for(const auto& i : universe){
                finalPos += i.second.position;
            }
            finalPos /= universe.size();
        }
        break;
    case Single:
        if(universe.isValid(following)){
            finalPos = universe[following].position;
            break;
        }
    default:
        finalPos = position;
    }

    camera = glm::translate(projection, glm::vec3(0.0f, 0.0f, -distance));
    camera = glm::rotate(camera, xrotation - 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    camera = glm::rotate(camera, zrotation, glm::vec3(0.0f, 0.0f, 1.0f));
    camera = glm::translate(camera, -finalPos);
    return camera;
}

Ray Camera::getRay(const glm::ivec2 &pos, const int &windowW, const int &windowH, bool normalize, float startDepth, float endDepth) const {
    Ray ray;

    glm::mat4 model;
    glm::vec3 windowCoord(pos.x, windowH - pos.y, startDepth);
    glm::vec4 viewport(0.0f, 0.0f, windowW, windowH);

    ray.origin = glm::unProject(windowCoord, model, camera, viewport);

    windowCoord.z = endDepth;

    ray.direction = ray.origin - glm::unProject(windowCoord, model, camera, viewport);

    if(normalize){
        ray.direction = glm::normalize(ray.direction);
    }

    return ray;
}

PlanetsUniverse::key_type Camera::selectUnder(const glm::ivec2 &pos, const int &windowW, const int &windowH){
    universe.resetSelected();
    float nearest = -std::numeric_limits<float>::max();

    Ray ray = getRay(pos, windowW, windowH, true);

    for(const auto& i : universe){
        glm::vec3 difference = i.second.position - ray.origin;
        float dot = glm::dot(difference, ray.direction);
        if(dot > nearest && (glm::length2(difference) - dot * dot) <= (i.second.radius() * i.second.radius())) {
            universe.selected = i.first;
            nearest = dot;
        }
    }
}

void Camera::followNext(){
    if(!universe.isEmpty()){
        followingState = Single;
        PlanetsUniverse::const_iterator current = universe.find(following);

        if(current == universe.cend()){
            current = universe.cbegin();
        }else if(++current == universe.cend()){
            current = universe.cbegin();
        }

        following = current->first;
    }
}

void Camera::followPrevious(){
    if(!universe.isEmpty()){
        followingState = Single;
        PlanetsUniverse::const_iterator current = universe.find(following);

        if(current == universe.cend()){
            current = universe.cbegin();
        }else{
            if(current == universe.cbegin()){
                current = universe.cend();
            }
            --current;
        }

        following = current->first;
    }
}

void Camera::followSelection(){
    following = universe.selected;
    followingState = Single;
}

void Camera::clearFollow(){
    following = 0;
    followingState = FollowNone;
}

void Camera::followPlainAverage(){
    followingState = PlainAverage;
}

void Camera::followWeightedAverage(){
    followingState = WeightedAverage;
}
