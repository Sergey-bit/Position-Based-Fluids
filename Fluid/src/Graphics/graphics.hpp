#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <glm/glm.hpp>
#include <GL/GLU.h>
#include <string>
#include <fstream>
#include <iostream>

#include "../settings.hpp"
#include "../math/minmath.hpp"
#include "../PBF/particles.hpp"


std::string LoadShader(const std::string& path);

// Pure openGL
GLuint CompileShader(const GLuint& type, const std::string& shaderCode, bool& success);
bool InitGL(GLuint& gProgramID);
void ShadeScreen(GLuint& gVBO, GLuint& gIBO, GLint& gVertexPos2DLocation);
void PassUniforms(GLuint& prog, GLuint& UBO, GLint& blockSize);
void setupViewSettingsAndData(GLuint& buffer, GLuint& prog, GLint& blockSize);
void PassUniformConstants(GLuint& prog);

// SDL_GL
void SetAttributes();
void Init(SDL_Window*& w, SDL_GLContext& context, GLuint& gProgramID);

// SDL
SDL_Window* CreateWindow(const char* name);
void QuitSDL(SDL_Window*& w);

#endif