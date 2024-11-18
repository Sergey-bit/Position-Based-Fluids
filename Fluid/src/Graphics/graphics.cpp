#include "graphics.hpp"

std::string LoadShader(const std::string& path)
{
    std::ifstream file(path);
    if (file.is_open())
    {
        std::string text, line;
        while (!file.eof())
        {
            std::getline(file, line);
            text += line + "\n";
        }
        file.close();
        return text;
    }
    else
    {
        std::cout << "Failed to load shader" << std::endl;
        exit(1);
    }
}

// Pure openGL settings
GLuint CompileShader(const GLuint& type, const std::string& shaderCode, bool& success)
{
    GLuint shader = glCreateShader(type);
    GLchar* code = (GLchar*)shaderCode.c_str();
    GLint vShaderCompiled = GL_FALSE;

    glShaderSource(shader, 1, &code, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &vShaderCompiled);
    success = vShaderCompiled == GL_TRUE;

    if (!success)
    {
        char log[500];
        GLsizei s;
        glGetShaderInfoLog(shader, 500, &s, log);
        std::cout << "Failed to compile shader with type "
            << (unsigned)type << " " << log << std::endl;
    }
    return shader;
}
bool InitGL(GLuint& gProgramID)
{
    bool success = true;

    gProgramID = glCreateProgram();
    GLint programSuccess = GL_TRUE;

    std::string vsCode = LoadShader("shaders/vertex.shader");
    std::string fsCode = LoadShader("shaders/fragment.shader");

    GLuint vsID = CompileShader(GL_VERTEX_SHADER, vsCode, success);
    glAttachShader(gProgramID, vsID);
    if (!success) return success;

    GLuint fsID = CompileShader(GL_FRAGMENT_SHADER, fsCode, success);
    glAttachShader(gProgramID, fsID);
    if (!success) return success;

    glLinkProgram(gProgramID);
    glGetProgramiv(gProgramID, GL_LINK_STATUS, &programSuccess);

    if (programSuccess != GL_TRUE)
    {
        std::cout << "Error linking program " << gProgramID << "!\n";
        success = false;
    }
    return success;
}
void ShadeScreen(GLuint& gVBO, GLuint& gVAO, GLint& gVertexPos2DLocation)
{
    //VBO data
    GLfloat vertexData[] =
    {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };

    glGenVertexArrays(1, &gVAO);
    glBindVertexArray(gVAO);

    //Create VBO
    glGenBuffers(1, &gVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gVBO);
    glBufferData(GL_ARRAY_BUFFER, 2 * 4 * sizeof(GLfloat), vertexData, GL_STATIC_DRAW);
    glEnableVertexAttribArray(gVertexPos2DLocation);
    glVertexAttribPointer(gVertexPos2DLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), NULL);
    glBindVertexArray(0);
    glDisableVertexAttribArray(gVertexPos2DLocation);
}
void PassUniformConstants(GLuint& prog)
{
    GLint iResolutionLocation = glGetUniformLocation(prog, "iResolution");
    glUniform2f(iResolutionLocation, static_cast<float>(WWIDTH), static_cast<float>(WHEIGHT));

    GLint massLocation = glGetUniformLocation(prog, "mass");
    glUniform1f(massLocation, mass);

    GLint particlesCountLocation = glGetUniformLocation(prog, "particleCount");
    glUniform1i(particlesCountLocation, PARTICLES_NUMBER);

    GLint scaleLocation = glGetUniformLocation(prog, "scale");
    glUniform1f(scaleLocation, scale);

    GLint targetDensityLocation = glGetUniformLocation(prog, "targetDensity");
    glUniform1f(targetDensityLocation, targetDensity);

    GLint particleRadiusLocation = glGetUniformLocation(prog, "particleRadius");
    glUniform1f(particleRadiusLocation, radius);

    GLint particleInfluenceRadiusLocation = glGetUniformLocation(prog, "particleInfluenceRadius");
    glUniform1f(particleInfluenceRadiusLocation, influenceRadius);
}
void PassUniforms(GLuint& prog, GLuint& UBO, GLint& blockSize)
{
    GLint iResolutionLocation = glGetUniformLocation(prog, "iResolution");
    glUniform2f(iResolutionLocation, static_cast<float>(WWIDTH), static_cast<float>(WHEIGHT));

    GLint massLocation = glGetUniformLocation(prog, "mass");
    glUniform1f(massLocation, mass);

    GLint particlesCountLocation = glGetUniformLocation(prog, "particleCount");
    glUniform1i(particlesCountLocation, PARTICLES_NUMBER);

    GLint scaleLocation = glGetUniformLocation(prog, "scale");
    glUniform1f(scaleLocation, scale);

    GLint targetDensityLocation = glGetUniformLocation(prog, "targetDensity");
    glUniform1f(targetDensityLocation, targetDensity);

    std::vector<GLfloat> data(blockSize / 4);
    for (size_t i = 0; i < PARTICLES_NUMBER; ++i)
    {
        data[i * 4] = particles.centers[i].x;
        data[i * 4 + 1] = particles.centers[i].y;
        data[i * 4 + 2] = 0.0;
        data[i * 4 + 3] = 0.0;
    }

    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, blockSize, data.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    GLint particleRadiusLocation = glGetUniformLocation(prog, "particleRadius");
    glUniform1f(particleRadiusLocation, radius);

    GLint particleInfluenceRadiusLocation = glGetUniformLocation(prog, "particleInfluenceRadius");
    glUniform1f(particleInfluenceRadiusLocation, influenceRadius);
}
void setupViewSettingsAndData(GLuint& buffer, GLuint& prog, GLint& blockSize)
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glViewport(0, 0, WWIDTH, WHEIGHT);
    glClearColor(0.2f, 0.2f, 0.2f, 1.f);

    // particles
    GLint particleBlockLocation = glGetUniformBlockIndex(prog, "ParticlesBlock");

    glUniformBlockBinding(prog, particleBlockLocation, 0);
    glGetActiveUniformBlockiv(prog, particleBlockLocation, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    glCreateBuffers(1, &buffer);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, 0, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


// SDL_GL
void SetAttributes()
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
}
void Init(SDL_Window*& w, SDL_GLContext& context, GLuint& gProgramID)
{
    const char* const name = "Fluid Simulation";

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL could not initialize! SDL_Error: "
            << SDL_GetError() << std::endl;;
        exit(1);
    }

    SetAttributes();

    w = CreateWindow(name);
    context = SDL_GL_CreateContext(w);
    if (context == NULL)
        std::cout << SDL_GetError() << std::endl;
    glewExperimental = GL_TRUE;

    if (SDL_GL_SetSwapInterval(1) < 0)
    {
        std::cout << "Warning: Unable to set VSync! SDL Error: "
            << SDL_GetError() << std::endl;
    }
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cout << "Failed to initialize openGL version 4.1 "
            << glewGetErrorString(err) << std::endl;
        exit(1);
    }
    if (!InitGL(gProgramID))
    {
        std::cout << "Unable to initialize OpenGL!\n";
        exit(1);
    }
    
}

// SDL
SDL_Window* CreateWindow(const char* name)
{
    SDL_Window* window = SDL_CreateWindow(
        name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WWIDTH, WHEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );
    if (window == NULL)
    {
        throw "Window could not be created! SDL_Error: %s\n", SDL_GetError();
    }

    return window;
}
void QuitSDL(SDL_Window*& w)
{
    SDL_DestroyWindow(w);
    SDL_Quit();
}

