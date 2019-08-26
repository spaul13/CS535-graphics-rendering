#ifndef _COMMON_HPP
#define _COMMON_HPP

#include <string>
#include <vector>
#include <glad/glad.h>

GLuint compileShader(GLenum type, std::string filename, std::string prepend = "");
GLuint linkProgram(std::vector<GLuint> shaders);

#endif