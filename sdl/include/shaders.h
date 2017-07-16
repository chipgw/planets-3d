#pragma once

#ifdef PLANETS3D_WITH_GLEW
#include <GL/glew.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>
#endif

enum VertexAttrib {
    vertex,
    tangent,
    uv
};

/* Functions for compiling and linking shaders. */
GLuint compileShader(const unsigned char *source, GLenum shaderType);
GLuint linkShaderProgram(GLuint vsh, GLuint fsh);
