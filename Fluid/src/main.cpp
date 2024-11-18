#include <iostream>
#include "settings.hpp"
#include "PBF/particles.hpp"
#include "Graphics/graphics.hpp"


void Update(GLuint&, GLuint&, GLint&);

void MainLoop(SDL_Window*, SDL_GLContext&, GLuint&);

// Event handler
void Input(bool& quit);


int main(int argc, char* args[])
{
    SDL_Window* window = NULL;
    SDL_GLContext context = NULL;
    GLuint gProgramID = NULL;

    Init(window, context, gProgramID);
    MainLoop(window, context, gProgramID);
    QuitSDL(window);

    return 0;
}


void Input(bool& quit)
{
    static bool pressed = false;
    SDL_Event e;

    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
        {
            quit = true;
        }
        else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
        {
            interactionInputStrength = 100.0;
            pressed = true;
        }
        else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT)
        {
            interactionInputStrength = -100.0;
            pressed = true;
        }
        else if (e.type == SDL_MOUSEBUTTONUP)
        {
            interactionInputStrength = 0.0;
            pressed = false;
        }
        if (e.type == SDL_MOUSEWHEEL && pressed)
        {
            interactionInputStrength += e.wheel.y * 10.0;
        }

        if (e.type == SDL_MOUSEMOTION && pressed)
        {
            int x, y;
            SDL_GetMouseState(&x, &y);

            interactionInputPoint.x = x;
            interactionInputPoint.y = WHEIGHT - y;
        }
    }
}
void MainLoop(SDL_Window* w, SDL_GLContext& context, GLuint& prog)
{
    bool quit = false;

    GLuint gVBO, gVAO, fUBO;
    GLint blockSize;

    initParticles();
    setupViewSettingsAndData(fUBO, prog, blockSize);

    GLint gVertexPos2DLocation = glGetAttribLocation(prog, "position");
    if (gVertexPos2DLocation == -1)
    {
        std::cout << "LVertexPos2D is not a valid GLSL program variable!\n";
        exit(1);
    }

    ShadeScreen(gVBO, gVAO, gVertexPos2DLocation);
    glUseProgram(prog);

    PassUniformConstants(prog);
    glBindVertexArray(gVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gVBO);

    while (!quit)
    {
        Input(quit);

        // clearing the scene
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update Image View
        Update(prog, fUBO, blockSize);

        // Swap buffers
        SDL_GL_SwapWindow(w);
    }

    glUseProgram(NULL);
}
void Update(GLuint& prog, GLuint& UBO, GLint& blockSize)
{
    // Process
    particlesUpdate();

    // Draw
    PassUniforms(prog, UBO, blockSize);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

}

