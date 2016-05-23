// Local Headers
#include "glitter.hpp"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Standard Headers
#include <chrono>
#include <cstdio>
#include <cstdlib>

// Shader sources
const GLchar* vertexSource =
    "#version 150 core\n"
    "in vec2 position;"
    "in vec3 color;"
    "in vec2 texcoord;"
    "out vec3 Color;"
    "out vec2 Texcoord;"
    "uniform mat4 model;"
    "uniform mat4 view;"
    "uniform mat4 proj;"
    "void main()"
    "{"
    "    Color = color;"
    "    Texcoord = texcoord;"
    "    gl_Position = proj * view * model * vec4(position, 0.0, 1.0);"
    "}";
const GLchar* fragmentSource =
    "#version 150 core\n"
    "in vec3 Color;"
    "in vec2 Texcoord;"
    "out vec4 outColor;"
    "uniform float mixAmount;"
    "uniform sampler2D texKitten;"
    "void main()"
    "{"
    "    if (Texcoord.y < 0.5)"
    "        outColor = texture(texKitten, Texcoord);"
    "    else"
    "        outColor = texture(texKitten,"
    "            vec2(Texcoord.x + 0.05*(sin(50.0*Texcoord.y)), 1.0 - Texcoord.y)"
    "        ) * vec4(0.7, 0.7, 1.0, 1.0);"
    "}";

int main(int argc, char * argv[]) {
    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    auto mWindow = glfwCreateWindow(mWidth, mHeight, "LearnOpenGL", nullptr, nullptr);

    // Check for Valid Context
    if (mWindow == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    gladLoadGL();
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

    // Vertex Array Objects (VAO): store links between attributes and VBOs with raw vertex data
    // create and bound VAO at start b/c buffers/element buffers bound before will be ignored
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao); // future `glVertexAttribPointer` will stored in this VAO

    // Create Vertex Buffer Object (VBO) and copy data over
    GLuint vbo; // uint descriptor to Vertex Buffer Object
    glGenBuffers(1, &vbo); // Generate 1 buffer

    GLfloat vertices[] = {
    //  Position      Color             Texcoords
        -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top-left
         0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Top-right
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
        -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
    };
    glBindBuffer(GL_ARRAY_BUFFER, vbo); // makes VBO active array_buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // copy data

    // Add element buffer to control order of rengering
    GLuint ebo;
    glGenBuffers(1, &ebo);

    GLuint elements[] = {
        0, 1, 2,
        2, 3, 0
    };
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    // Create/compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); // int descriptor to shader object
    glShaderSource(vertexShader, 1, &vertexSource, NULL); // makes active
    glCompileShader(vertexShader);

    // Create/compile fragmentshader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    // Link shaders into shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor"); // fragment shader allowed to write multiple buffers
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    // Specify layout of vertex data (saved to bound VAO)
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7*sizeof(GLfloat), 0);

    GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
    glEnableVertexAttribArray(colAttrib);
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7*sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));

    GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7*sizeof(GLfloat), (void*)(5*sizeof(GLfloat)));

    // Specify texture
    GLuint textures[1];
    glGenTextures(1, textures);

    int width, height, ncomp;
    unsigned char* image;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    image = stbi_load("sample.png", &width, &height, &ncomp, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    stbi_image_free(image);
    glUniform1i(glGetUniformLocation(shaderProgram, "texKitten"), 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    // Model matrix
    glm::mat4 model;
    GLint uniTrans = glGetUniformLocation(shaderProgram, "model");

    // View matrix
    glm::mat4 view = glm::lookAt(
            glm::vec3(1.2f, 1.2f, 1.2f), // position of the camera
            glm::vec3(0.0f, 0.0f, 0.0f), // point to be centered on-screen
            glm::vec3(0.0f, 0.0f, 1.0f)  // up axis (+z direction)
    );
    GLint uniView = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

    // Projection matrix
    glm::mat4 proj = glm::perspective(
            glm::radians(45.0f), // vertical field-of-view
            800.0f / 600.0f, // aspect ratio (width/height)
            1.0f, // near plane
            10.0f); // far plane
    GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

    // Time
    auto t_start = std::chrono::high_resolution_clock::now();

    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

        // Calculate time
        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        // Update transform
        model= glm::rotate(
                model,
                0.001f * time * glm::radians(180.0f),
                glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(model));

        // Background Fill Color
        glClearColor(0.0f, 0.0, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw a triangle from the 3 vertices
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }   glfwTerminate();
    return EXIT_SUCCESS;
}
