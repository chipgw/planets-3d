#include "camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

Camera::Camera(PlanetsUniverse &u) : universe(u), followingState(FollowNone) {
    reset();
}

void Camera::bound(){
    distance = glm::clamp(distance, 10.0f, 1.0e4f);
    xrotation = glm::clamp(xrotation, -glm::half_pi<float>(), glm::half_pi<float>());
    zrotation = glm::mod(zrotation, glm::pi<float>() * 2.0f);
}

void Camera::reset(){
    distance = 100.0f;
    xrotation = glm::quarter_pi<float>();
    zrotation = 0.0f;
}

void Camera::resizeViewport(const float &width, const float &height){
    viewport = glm::vec4(0.0f, 0.0f, width, height);

    projection = glm::perspective(glm::quarter_pi<float>(), width / height, 0.1f, 1.0e6f);
}

const glm::mat4 &Camera::setup(){
    switch(followingState){
    case WeightedAverage:
        if(universe.size() != 0){
            position = glm::vec3();
            float totalMass = 0.0f;

            for(const auto& i : universe){
                position += i.second.position * i.second.mass();
                totalMass += i.second.mass();
            }
            position /= totalMass;
        }
        break;
    case PlainAverage:
        if(universe.size() != 0){
            position = glm::vec3();
            for(const auto& i : universe){
                position += i.second.position;
            }
            position /= universe.size();
        }
        break;
    case Single:
        if(universe.isValid(following)){
            position = universe[following].position;
            break;
        }
    }

    camera = glm::translate(projection, glm::vec3(0.0f, 0.0f, -distance));
    camera = glm::rotate(camera, xrotation - glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));
    camera = glm::rotate(camera, zrotation, glm::vec3(0.0f, 0.0f, 1.0f));
    camera = glm::translate(camera, -position);
    return camera;
}

Ray Camera::getRay(const glm::ivec2 &pos, float startDepth, float endDepth) const {
    Ray ray;

    glm::mat4 model;
    glm::vec3 windowCoord(pos.x, viewport.w - pos.y, startDepth);

    ray.origin = glm::unProject(windowCoord, model, camera, viewport);

    windowCoord.z = endDepth;

    ray.direction = glm::normalize(ray.origin - glm::unProject(windowCoord, model, camera, viewport));

    return ray;
}

PlanetsUniverse::key_type Camera::selectUnder(const glm::ivec2 &pos){
    universe.resetSelected();
    float nearest = -std::numeric_limits<float>::max();

    Ray ray = getRay(pos);

    for(const auto& i : universe){
        glm::vec3 difference = i.second.position - ray.origin;
        float dot = glm::dot(difference, ray.direction);
        if(dot > nearest && (glm::length2(difference) - dot * dot) <= (i.second.radius() * i.second.radius())) {
            universe.selected = i.first;
            nearest = dot;
        }
    }

	return universe.selected;
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
