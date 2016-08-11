#include "shaders.h"
#include <cstdio>

/* Embed shader source code as c-style strings. */

const char* color_vertex_src =
        #include "../shaders/color.vsh"
        ;
const char* color_fragment_src =
        #include "../shaders/color.fsh"
        ;
const char* texture_vertex_src =
        #include "../shaders/texture.vsh"
        ;
const char* texture_fragment_src =
        #include "../shaders/texture.fsh"
        ;

/* Compile a shader from source string. */
GLuint compileShader(const char* source, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);

    glShaderSource(shader, 1, (const GLchar**)&source, 0);

    glCompileShader(shader);

    int isCompiled, maxLength;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    if (maxLength > 1) {
        char* infoLog = new char[maxLength];

        glGetShaderInfoLog(shader, maxLength, &maxLength, infoLog);

        if (isCompiled == GL_FALSE) {
            printf("ERROR: failed to compile shader!\n\tLog: %s", infoLog);
            delete[] infoLog;

            return 0;
        } else {
            printf("INFO: succesfully compiled shader.\n\tLog: %s", infoLog);
            delete[] infoLog;
        }
    } else if (isCompiled == GL_FALSE) {
        printf("ERROR: failed to compile shader! No log availible.\n");
        return 0;
    }
    return shader;
}

/* Link a vertex and fragment shader into a shader program. */
GLuint linkShaderProgram(GLuint vsh, GLuint fsh) {
    GLuint program = glCreateProgram();

    glAttachShader(program, vsh);
    glAttachShader(program, fsh);

    glBindAttribLocation(program, vertex,   "vertex");
    glBindAttribLocation(program, normal,   "normal");
    glBindAttribLocation(program, tangent,  "tangent");
    glBindAttribLocation(program, uv,       "uv");

    glLinkProgram(program);

    int isLinked, maxLength;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

    if (maxLength > 1) {
        char* infoLog = new char[maxLength];

        glGetProgramInfoLog(program, maxLength, &maxLength, infoLog);

        if (isLinked == GL_FALSE) {
            printf("ERROR: failed to link shader program!\n\tLog: %s", infoLog);
            delete[] infoLog;

            return 0;
        } else {
            printf("INFO: succesfully linked shader.\n\tLog: %s", infoLog);
            delete[] infoLog;
        }
    } else if (isLinked == GL_FALSE) {
        printf("ERROR: failed to link shader program! No log availible.\n");
        return 0;
    }

    return program;
}
