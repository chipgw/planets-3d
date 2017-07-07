#include <spheregenerator.h>
#include <grid.h>
#include <camera.h>
#include "glbindings.h"

void bindGrid(Grid& grid, bool update) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    if (update)
        /* Only update the buffer when the point data changes. */
        glBufferData(GL_ARRAY_BUFFER, grid.points.size() * sizeof(glm::vec2), grid.points.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(vertex, 2, GL_FLOAT, GL_FALSE, 0, 0);
}
void drawGrid(Grid& grid) {
    glDrawArrays(GL_LINES, 0, GLsizei(grid.points.size()));
}

EMSCRIPTEN_BINDINGS(grid) {
    emscripten::class_<Grid>("Grid")
            .constructor()
            .function("bind",       &bindGrid)
            .function("draw",       &drawGrid)
            .function("update",     &Grid::update)
            .property("color",      &Grid::color)
            .property("range",      &Grid::range)
            .property("scale",      &Grid::scale)
            .property("alphafac",   &Grid::alphafac)
            .property("enabled",    &Grid::draw)
            ;
}
