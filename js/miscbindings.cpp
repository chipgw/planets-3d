#include <planetsuniverse.h>
#include <camera.h>
#include <bind.h>
#include <glm/vec2.hpp>

int bench();

EMSCRIPTEN_BINDINGS(bench) {
    emscripten::function("bench", &bench);
}

EMSCRIPTEN_BINDINGS(vectors) {
    /* TODO - Do we need functions for vectors? */
    emscripten::value_array<glm::vec3>("vec3")
            .element(&glm::vec3::x)
            .element(&glm::vec3::y)
            .element(&glm::vec3::z)
            ;
    /* Used for colors. */
    emscripten::value_array<glm::vec4>("vec4")
            .element(&glm::vec4::r)
            .element(&glm::vec4::g)
            .element(&glm::vec4::b)
            .element(&glm::vec4::a)
            ;
    emscripten::value_array<glm::ivec2>("ivec2")
            .element(&glm::ivec2::x)
            .element(&glm::ivec2::y)
            ;
}

template<int x, int y> float getMatElement(const glm::mat4& mat) {
    return mat[x][y];
}

template<int x, int y> void setMatElement(glm::mat4& mat, float val) {
    mat[x][y] = val;
}

glm::mat4 getIdentityMatrix() { return glm::mat4(); }

/* TODO - This seems like it wouldn't be the best way to do this,
 * plus I don't know if matrix manipulation functions will need to be called in JS... */
EMSCRIPTEN_BINDINGS(mat4) {
    emscripten::value_array<glm::mat4>("mat4")
            .element(&getMatElement<0, 0>, &setMatElement<0, 0>)
            .element(&getMatElement<0, 1>, &setMatElement<0, 1>)
            .element(&getMatElement<0, 2>, &setMatElement<0, 2>)
            .element(&getMatElement<0, 3>, &setMatElement<0, 3>)
            .element(&getMatElement<1, 0>, &setMatElement<1, 0>)
            .element(&getMatElement<1, 1>, &setMatElement<1, 1>)
            .element(&getMatElement<1, 2>, &setMatElement<1, 2>)
            .element(&getMatElement<1, 3>, &setMatElement<1, 3>)
            .element(&getMatElement<2, 0>, &setMatElement<2, 0>)
            .element(&getMatElement<2, 1>, &setMatElement<2, 1>)
            .element(&getMatElement<2, 2>, &setMatElement<2, 2>)
            .element(&getMatElement<2, 3>, &setMatElement<2, 3>)
            .element(&getMatElement<3, 0>, &setMatElement<3, 0>)
            .element(&getMatElement<3, 1>, &setMatElement<3, 1>)
            .element(&getMatElement<3, 2>, &setMatElement<3, 2>)
            .element(&getMatElement<3, 3>, &setMatElement<3, 3>)
            ;
    emscripten::function("getIdentityMatrix", &getIdentityMatrix);
}

glm::vec3 getLightDir(Camera& camera) {
    const glm::vec3 light = glm::vec3(0.57735f);
    return glm::vec3(camera.view * glm::vec4(light, 0.0f));
}

EMSCRIPTEN_BINDINGS(camera) {
    emscripten::class_<Camera>("Camera")
            .constructor<PlanetsUniverse&>()
            .function("bound",                  &Camera::bound)
            .function("clearFollow",            &Camera::clearFollow)
            .function("followNext",             &Camera::followNext)
            .function("followPrevious",         &Camera::followPrevious)
            .function("followPlainAverage",     &Camera::followPlainAverage)
            .function("followWeightedAverage",  &Camera::followWeightedAverage)
            .function("followSelection",        &Camera::followSelection)
            .function("reset",                  &Camera::reset)
            .function("resizeViewport",         &Camera::resizeViewport)
            .function("selectUnder",            &Camera::selectUnder)
            .function("setup",                  &Camera::setup)
            .function("getLightDir",            &getLightDir)
            .property("position",               &Camera::position)
            .property("distance",               &Camera::distance)
            .property("xrotation",              &Camera::xrotation)
            .property("zrotation",              &Camera::zrotation)
            .property("viewMat",                &Camera::view)
            ;
}
