#include "shaders.h"
#include <cstdio>

/* Compile a shader from source string. */
GLuint compileShader(const unsigned char* source, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);

    glShaderSource(shader, 1, (const GLchar**)&source, 0);

    glCompileShader(shader);

    /* Get info about compile status from OpenGL. */
    int isCompiled, maxLength;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    if (maxLength > 1) {
        /* If there's a log, we print it. */
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

    /* These can flagged for deletion now, as once they are attached to
     * a program OpenGL will keep them alive as long as they are needed. */
    glDeleteShader(vsh);
    glDeleteShader(fsh);

    /* It doesn't matter if the attributes don't exist in given shaders,
     * but if they do make sure they are bound to the correct location. */
    glBindAttribLocation(program, vertex,   "vertex");
    glBindAttribLocation(program, tangent,  "tangent");
    glBindAttribLocation(program, uv,       "uv");

    glLinkProgram(program);

    /* Get info about link status from OpenGL. */
    int isLinked, maxLength;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

    if (maxLength > 1) {
        /* If there's a log, we print it. */
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
