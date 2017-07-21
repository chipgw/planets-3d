#include "spheregenerator.h"
#include <cstring>

Circle::Circle(uint32_t slices) : vertexCount(slices), lineCount(slices * 2) {
    verts = new glm::vec3[vertexCount];
    lines = new uint32_t[lineCount];

    /* The amount of rotation needed for each slice. */
    float step = (2.0f * glm::pi<float>()) / slices;

    /* Keep track of the next index in the line index buffer. */
    uint32_t currentLine = 0;

    for (uint32_t current = 0; current < slices; ++current) {
        /* Calculate the coordinate of the vertex. */
        verts[current][0] = glm::cos(current * step);
        verts[current][1] = glm::sin(current * step);
        verts[current][2] = 0.0f;

        /* Add the current vertex to the index buffer. */
        lines[currentLine++] = current;
        /* If we're on the last slice, connect it to the first vertex, otherwise connect it to the next one. */
        lines[currentLine++] = current == (slices - 1) ? 0 : current + 1;
    }
}

Circle::~Circle() {
    delete[] verts;
    delete[] lines;
}

Sphere::Sphere(uint32_t slices, uint32_t stacks) : vertexCount((slices + 1) * (stacks + 1)), triangleCount(slices * stacks * 6), lineCount(slices * stacks * 4) {
    verts = new Vertex[vertexCount];
    triangles = new uint32_t[triangleCount];
    lines = new uint32_t[lineCount];

    /* The amount of rotation needed for each stack, ranging from pole to pole. */
    float vstep = glm::pi<float>() / stacks;
    /* The amount of rotation needed for each slice, ranging all the way around the sphere. */
    float hstep = (2.0f * glm::pi<float>()) / slices;

    /* Keep track of the next index in the line index buffer. */
    uint32_t currentTriangle = 0;
    uint32_t currentLine = 0;

    /* The offset for the index to connect to in the next stack.  */
    const uint32_t w = slices + 1;

    for (uint32_t v = 0; v <= stacks; ++v) {
        /* Calculate the height and radius of the stack. */
        float z = glm::cos(v * vstep);
        float r = glm::sin(v * vstep);

        for (uint32_t h = 0; h <= slices; ++h) {
            /* The index of the current vertex. */
            uint32_t current = v * w + h;

            /* Make a circle with the radiusof the current stack. */
            verts[current].position.x = glm::cos(h * hstep) * r;
            verts[current].position.y = glm::sin(h * hstep) * r;
            /* Well this is easy... */
            verts[current].position.z = z;

            /* Tangents are 90 degrees off from the circle coordinate with no z. */
            verts[current].tangent.x = -glm::sin(h * hstep);
            verts[current].tangent.y = glm::cos(h * hstep);
            verts[current].tangent.z = 0;

            /* The texture coordinate is simply how far along we are in our slizes/stacks. */
            verts[current].uv.x = float(h) / float(slices);
            verts[current].uv.y = float(v) / float(stacks);

            if (h != slices && v != stacks) {
                /* A triangle with the current vertex, the next one, and the one above it. */
                triangles[currentTriangle++] = current;
                triangles[currentTriangle++] = current + w;
                triangles[currentTriangle++] = current + 1;

                /* A triangle with the next vertex, the one above it, and the one above the current. */
                triangles[currentTriangle++] = current + w + 1;
                triangles[currentTriangle++] = current + 1;
                triangles[currentTriangle++] = current + w;

                /* A line connecting to the next vertex in the list. */
                lines[currentLine++] = current;
                lines[currentLine++] = current + 1;

                /* A line connecting to the vertex above this one. */
                lines[currentLine++] = current;
                lines[currentLine++] = current + w;
            }
        }
    }
}

Sphere::~Sphere() {
    delete[] verts;
    delete[] triangles;
    delete[] lines;
}

namespace Icosahedron {

static constexpr float A = 0.89442f;
static constexpr float B = 0.27639f;
static constexpr float C = 0.85064f;
static constexpr float D = 0.7236f;
static constexpr float E = 0.52572f;
static constexpr float Z = 0.44721f;
static constexpr float N = 0.0f;
static constexpr float U = 1.0f;

static const glm::vec3 verts[] = {
    {N,N,U},
    { A,N, Z}, {B,C,Z}, {-D,E,Z}, {-D,-E,Z}, {B,-C,Z},
    {-A,N,-Z}, {-B,C,-Z}, {D,E,-Z}, {D,-E,-Z}, {-B,-C,-Z},
    {N,N,-U}
};

static const uint32_t triangles[] = {
    0,1,2,  0,2,3,  0,3,4,  0,4,5,   0,5,1,
    1,9,8,  2,8,7,  3,7,6,  4,6,10,  5,10,9,
    1,8,2,  2,7,3,  3,6,4,  4,10,5,  5,9,1,
    6,7,11, 7,8,11, 8,9,11, 9,10,11, 10,6,11
};

static const uint32_t lines[] {
    0,1,  0,2,  0,3,  0,4,  0,5,
    1,2,  2,3,  3,4,  4,5,  5,1,
    1,9, 1,8, 2,8, 2,7, 3,7, 3,6, 4,6, 4,10, 5,10, 5,9,
    6,7, 7,8, 8,9, 9,10, 10,6,
    6,11, 7,11, 8,11, 9,11, 10,11
};

}

IcoSphere::IcoSphere(uint8_t divisions) : vertexCount(glm::pow(4, divisions) * 10 + 2), triangleCount((vertexCount - 2) * 6), lineCount(triangleCount) {
    verts = new Vertex[vertexCount];
    triangles = new uint32_t[triangleCount];
    lines = new uint32_t[lineCount];

    if (divisions == 0)
        initNoSubdiv();
    else
        subdivide(divisions - 1);
}

IcoSphere::~IcoSphere() {
    delete[] verts;
    delete[] triangles;
    delete[] lines;
}

#include <map>
typedef std::pair<uint32_t, uint32_t> Edge;
typedef std::map<Edge, uint32_t> DivisionCache;

/* Convert a spherical coordinate into a spheremaped UV coordinate. */
glm::vec2 posToUV(glm::vec3 pos) {
    return { glm::atan(pos.x, pos.y) / (2.0f*glm::pi<float>()) + 0.5f,
             0.5f - glm::asin(pos.z) / glm::pi<float>() };
}

/* Generate a tangent from the spherical coordinate. */
glm::vec3 posToTangent(glm::vec3 pos) {
    return fabs(pos.z) >= 1.0f ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::normalize(glm::vec3(-pos.y, pos.x, 0.0f));
}

uint32_t makeOrGetVertex(Edge edge, Vertex* verts, uint32_t* lines, DivisionCache& cache, uint32_t& currentVert, uint32_t& currentLine) {
    /* Make sure the edge is in the correct order. */
    if (edge.second < edge.first)
        edge = {edge.second, edge.first};

    /* Check to see if we've done this edge before. */
    if (cache.count(edge) > 0)
        return cache.at(edge);

    glm::vec3 v1 = verts[edge.first].position;
    glm::vec3 v2 = verts[edge.second].position;

    /* Get the average position of the two, and normalize it to put in on the sphere surface. */
    glm::vec3 vNew = glm::normalize((v1 + v2) * 0.5f);

    uint32_t index = currentVert++;

    verts[index].position = vNew;
    verts[index].tangent = posToTangent(vNew);
    verts[index].uv = posToUV(vNew);

    lines[currentLine++] = edge.first; lines[currentLine++] = index;
    lines[currentLine++] = index; lines[currentLine++] = edge.second;

    /* Remember this edge for the other triangle that uses it. */
    cache.insert({edge,index});

    return index;
}

void IcoSphere::subdivide(uint8_t divisions) {
    IcoSphere parent(divisions);

    memcpy(verts, parent.verts, parent.vertexCount * sizeof(Vertex));

    DivisionCache cache;

    /* t is the current triangle being read, the others are the index of the next element to insert. */
    uint32_t t = 0, currentTri = 0, currentLine = 0, currentVert = parent.vertexCount;

    /* Iterate over the parent sphere's triangles and subdivide them. */
    while (t < parent.triangleCount) {
        uint32_t triangle[3] = { parent.triangles[t++],
                                 parent.triangles[t++],
                                 parent.triangles[t++] };

        /* Divide each edge on the triangle. */
        uint32_t a = makeOrGetVertex({triangle[0], triangle[1]}, verts, lines, cache, currentVert, currentLine);
        uint32_t b = makeOrGetVertex({triangle[1], triangle[2]}, verts, lines, cache, currentVert, currentLine);
        uint32_t c = makeOrGetVertex({triangle[2], triangle[0]}, verts, lines, cache, currentVert, currentLine);

        /* Add four triangles to cover the full area of the old triangle. */
        triangles[currentTri++] = triangle[0]; triangles[currentTri++] = a; triangles[currentTri++] = c;
        triangles[currentTri++] = triangle[1]; triangles[currentTri++] = b; triangles[currentTri++] = a;
        triangles[currentTri++] = triangle[2]; triangles[currentTri++] = c; triangles[currentTri++] = b;
        triangles[currentTri++] = a; triangles[currentTri++] = b; triangles[currentTri++] = c;

        /* Add the inner lines, the outer lines are handled inside makeOrGetVertex() so that they aren't done twice. */
        lines[currentLine++] = a; lines[currentLine++] = b;
        lines[currentLine++] = b; lines[currentLine++] = c;
        lines[currentLine++] = c; lines[currentLine++] = a;
    }
}

void IcoSphere::initNoSubdiv() {
    memcpy(triangles, Icosahedron::triangles, triangleCount * sizeof(uint32_t));
    memcpy(lines, Icosahedron::lines, lineCount * sizeof(uint32_t));

    for (uint32_t v = 0; v < vertexCount; ++v) {
        verts[v].position = Icosahedron::verts[v];
        verts[v].tangent = posToTangent(verts[v].position);
        verts[v].uv = posToUV(verts[v].position);
    }
}
