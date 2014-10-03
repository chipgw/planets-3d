#ifndef PLACINGINTERFACE_H
#define PLACINGINTERFACE_H

#include <planet.h>
#include <camera.h>

class PlacingInterface{
    PlanetsUniverse &universe;

public:
    enum PlacingStep{
        NotPlacing,
        FreePositionXY,
        FreePositionZ,
        FreeVelocity,
        Firing,
        OrbitalPlanet,
        OrbitalPlane
    };

    PlacingStep step;
    Planet planet;
    glm::mat4 rotation;
    float orbitalRadius;

    float firingSpeed;
    float firingMass;

    bool handleMouseMove(const glm::ivec2 &pos, const glm::ivec2 &delta, const int &windowW, const int &windowH, const Camera &camera, bool &holdMouse);
    bool handleMouseClick(const glm::ivec2 &pos, const int &windowW, const int &windowH, const Camera &camera);
    bool handleMouseWheel(float delta);

    void beginInteractiveCreation();
    void enableFiringMode(bool enable);
    void beginOrbitalCreation();

    PlacingInterface(PlanetsUniverse &universe);
};

#endif // PLACINGINTERFACE_H
