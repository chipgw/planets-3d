#pragma once

#include "planet.h"
#include <glm/mat4x4.hpp>

class PlacingInterface {
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

    /* Which step are we on? */
    PlacingStep step = NotPlacing;

    /* Variables for Free and Orbital modes. */
    Planet planet;
    glm::mat4 rotation;
    float orbitalRadius;

    /* Variables for firing mode. */
    float firingSpeed;
    float firingMass = 25.0f;

    /* Handle mouse events. These functions return true if input was used,
     * false if input was not used and should be used for other stuff such as camera movement.
     * holdMouse is set to true if the mouse cursor should be hidden and kept still by the window manager. */
    EXPORT bool handleMouseMove(const glm::ivec2& pos, const glm::ivec2& delta, const Camera& camera, bool& holdMouse);
    EXPORT bool handleMouseClick(const glm::ivec2& pos, const Camera& camera);
    EXPORT bool handleMouseWheel(float delta);

    /* Will change the camera position! */
    EXPORT bool handleAnalogStick(const glm::vec2& pos, const bool& modifier, Camera& camera);

    /* Set the current placing mode.  */
    EXPORT void enableFiringMode(bool enable);
    EXPORT void beginInteractiveCreation();
    EXPORT void beginOrbitalCreation();

    EXPORT PlacingInterface(PlanetsUniverse& universe);

    EXPORT glm::mat4 getOrbitalCircleMat();
    EXPORT glm::mat4 getOrbitedCircleMat();
};
