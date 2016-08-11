#pragma once

#ifdef PLANETS3D_WITH_GLEW
#include <GL/glew.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>
#endif

/* Pointers to the embedded GLSL source code.
 * I wish there was some standard way to embed these... */
extern const char* color_vertex_src;
extern const char* color_fragment_src;
extern const char* texture_vertex_src;
extern const char* texture_fragment_src;

enum VertexAttrib {
    vertex,
    normal,
    tangent,
    uv
};

/* Functions for compiling and linking shaders. */
GLuint compileShader(const char* source, GLenum shaderType);
GLuint linkShaderProgram(GLuint vsh, GLuint fsh);
