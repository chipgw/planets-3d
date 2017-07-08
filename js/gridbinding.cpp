#include <spheregenerator.h>
#include <grid.h>
#include <camera.h>
#include "glbindings.h"

void updateGrid(Grid& grid, Camera& camera) {
    if (grid.update(camera))
        /* Only update the buffer when the point data changes. */
        glBufferData(GL_ARRAY_BUFFER, grid.points.size() * sizeof(glm::vec2), grid.points.data(), GL_DYNAMIC_DRAW);
}
int gridNumPoints(Grid& grid) {
    return grid.points.size();
}

EMSCRIPTEN_BINDINGS(grid) {
    emscripten::class_<Grid>("Grid")
            .constructor()
            .function("update",     &updateGrid)
            .function("numPoints",  &gridNumPoints)
            .property("color",      &Grid::color)
            .property("range",      &Grid::range)
            .property("scale",      &Grid::scale)
            .property("alphafac",   &Grid::alphafac)
            .property("enabled",    &Grid::draw)
            ;
}
