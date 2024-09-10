#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <glad/glad.h>
#include <battery/embed.hpp>

#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Free texture: https://opengameart.org/content/crate-5

struct AppContext
{
    SDL_Window *window;
    SDL_GLContext glcontext;
    SDL_AppResult app_quit = SDL_APP_CONTINUE;
};

const char *vertexShaderSource =
    "attribute vec2 aPosition;\n"
    "attribute vec2 aTexCoord;\n"
    "varying vec2 vTexCoord;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(aPosition, 0.0, 1.0);\n"
    "    vTexCoord = aTexCoord;\n"
    "}\n";

const char *fragmentShaderSource =
    "uniform sampler2D uSampler;\n"
    "varying vec2 vTexCoord;\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = texture2D(uSampler, vTexCoord);\n"
    "}\n";

// Helper function for creating shaders
GLuint createShader(const char *shaderSource, int shaderType)
{
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
        glDeleteShader(shader); // Don't leak the shader
        std::cout << &(errorLog[0]) << std::endl;
        std::cout << shaderSource << std::endl;
    }
    return shader;
}

// Helper function for creating a shader program
GLuint createShaderProgram()
{
    GLuint program = glCreateProgram();
    GLuint vShader = createShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fShader = createShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    glLinkProgram(program);
    glUseProgram(program);

    return program;
}

// Load a texture
GLuint createTexture(const unsigned char *inputData, size_t size, bool isAlphaChannel = false)
{
    int h_image, w_image, cnt;
    // unsigned char *data = stbi_load(path.c_str(), &w_image, &h_image, &cnt, 0);
    unsigned char *data = stbi_load_from_memory(inputData, size, &w_image, &h_image, &cnt, 0);
    if (data == NULL)
    {
        std::cout << "Failed to load an image: " << std::endl;
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // GL_NEAREST - for pixel graphics
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (isAlphaChannel)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w_image, h_image, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w_image, h_image, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
    }

    stbi_image_free(data);

    return texture;
}

// Load a triangle to the video card
void initVertexBuffers(GLuint program)
{
    float vertPositions[] = {
        -0.5f, -0.5f,
        0.5f, -0.5f,
        -0.5f, 0.5f,
        0.5f, 0.5f
    };
    GLuint vertPosBuffer;
    glGenBuffers(1, &vertPosBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertPosBuffer);
    int amount = sizeof(vertPositions) / sizeof(vertPositions[0]);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(GLfloat),
        vertPositions, GL_STATIC_DRAW);
    GLint aPositionLocation = glGetAttribLocation(program, "aPosition");
    glVertexAttribPointer(aPositionLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(aPositionLocation);

    float texCoords[] = {
        0.f, 1.f,
        1.f, 1.f,
        0.f, 0.f,
        1.f, 0.f
    };
    GLuint texCoordBuffer;
    glGenBuffers(1, &texCoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
    amount = sizeof(texCoords) / sizeof(texCoords[0]);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(GLfloat),
        texCoords, GL_STATIC_DRAW);
    GLint aTexCoordLocation = glGetAttribLocation(program, "aTexCoord");
    glVertexAttribPointer(aTexCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(aTexCoordLocation);
}

SDL_AppResult SDL_Fail()
{
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    // init the library, here we make a window so we only need the Video capabilities.
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        return SDL_Fail();
    }

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1); // Enable MULTISAMPLE
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2); // can be 2, 4, 8 or 16

    // Create a window
    SDL_Window *window = SDL_CreateWindow("SDL3, OpenGL 2.1, C++", 400, 400,
        SDL_WINDOW_OPENGL); // | SDL_WINDOW_RESIZABLE
    if (!window)
    {
        return SDL_Fail();
    }

    SDL_GLContext glcontext = SDL_GL_CreateContext(window);
    if (!glcontext)
    {
        return SDL_Fail();
    }

    if (!gladLoadGL())
    {
        std::cout << "Failed to load OpenGL functions" << std::endl;
        return SDL_APP_FAILURE;
    }

    GLuint program = createShaderProgram();
    initVertexBuffers(program);

    GLint uSamplerLocation = glGetUniformLocation(program, "uSampler");
    glUniform1i(uSamplerLocation, 0);

    const unsigned char *data = (const unsigned char*)b::embed<"assets/crate.png">().data();
    size_t size = b::embed<"assets/crate.png">().size();
    createTexture(data, size, true);

    SDL_ShowWindow(window);

    // Set up the application data
    *appstate = new AppContext {
        window,
        glcontext,
    };

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, const SDL_Event *event)
{
    auto *app = (AppContext *)appstate;

    switch (event->type)
    {
        case SDL_EVENT_QUIT:
        {
            app->app_quit = SDL_APP_SUCCESS;
            break;
        }
        default:
        {
            break;
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    auto *app = (AppContext *)appstate;

    glClearColor(0.188f, 0.22f, 0.255f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    SDL_GL_SwapWindow(app->window);

    return app->app_quit;
}

void SDL_AppQuit(void *appstate)
{
    auto *app = (AppContext *)appstate;
    if (app)
    {
        SDL_GL_DestroyContext(app->glcontext);
        SDL_DestroyWindow(app->window);
        delete app;
    }

    SDL_Quit();
}
