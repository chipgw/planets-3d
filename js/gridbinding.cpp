#include <spheregenerator.h>
#include <grid.h>
#include <camera.h>
#include "glbindings.h"

void bindGrid(Grid& grid) {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glVertexAttribPointer(vertex, 2, GL_FLOAT, GL_FALSE, 0, grid.points.data());
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
