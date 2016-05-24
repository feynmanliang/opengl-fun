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
const GLchar* sceneVertexSource =
    "#version 150 core\n"
    "in vec3 position;"
    "in vec3 color;"
    "in vec2 texcoord;"
    "out vec3 Color;"
    "out vec2 Texcoord;"
    "uniform mat4 model;"
    "uniform mat4 view;"
    "uniform mat4 proj;"
    "uniform vec3 overrideColor;"
    "void main()"
    "{"
    "    Color = overrideColor * color;"
    "    Texcoord = texcoord;"
    "    gl_Position = proj * view * model * vec4(position, 1.0);"
    "}";
const GLchar* sceneFragmentSource =
    "#version 150 core\n"
    "in vec3 Color;"
    "in vec2 Texcoord;"
    "out vec4 outColor;"
    "uniform float mixAmount;"
    "uniform sampler2D texKitten;"
    "void main()"
    "{"
    "    outColor = vec4(Color, 1.0) * texture(texKitten, Texcoord);"
    "}";
const GLchar* screenVertexSource =
    "#version 150 core\n"
    "in vec2 position;"
    "in vec2 texcoord;"
    "void main()"
    "{"
    "    Texcoord = texcoord;"
    "    gl_Position = vec4(position, 0.0, 1.0);"
    "}";
const GLchar* fragmentSource2D =
    "#version 150 core\n"
    "in vec2 Texcoord;"
    "out vec4 outColor;"
    "uniform sampler2D texFramebuffer;"
    "void main()"
    "{"
    "    outColor = texture(texFramebuffer, Texcoord);"
    "}";

// Create a texture from an image file
GLuint loadTexture(const GLchar* path) {
    GLuint texture;
    glGenTextures(1, &texture);

    int width, height, ncomp;
    unsigned char* image;

    glBindTexture(GL_TEXTURE_2D, texture);
    image = stbi_load(path, &width, &height, &ncomp, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    stbi_image_free(image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	return texture;
}

int main(int argc, char * argv[]) {
    auto t_start = std::chrono::high_resolution_clock::now();

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

    // Create and bind framebuffer object
    GLuint frameBuffer;
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    // Create and use texture image for the color buffer of the new framebuffer
    GLuint texColorBuffer;
    glGenTextures(1, &texColorBuffer);
    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGB, mWidth, mHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

    // Renderbuffer Object to store combined depth/stencil buffers
    GLuint rboDepthStencil;
    glGenRenderbuffers(1, &rboDepthStencil);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mWidth, mHeight);
    glFramebufferRenderbuffer(
            GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencil);

    // Check that framebuffer successfully bound and attached
    GLint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("Framebuffer creation failed with status %#04x\n", status);
        return EXIT_FAILURE;
    }


    // Vertex Array Objects (VAO): store links between attributes and VBOs with raw vertex data
    // create and bound VAO at start b/c buffers/element buffers bound before will be ignored
    GLuint vaoCube, vaoQuad;
    glGenVertexArrays(1, &vaoCube);
    glGenVertexArrays(1, &vaoQuad);
    glBindVertexArray(vaoCube);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Create Vertex Buffer Object (VBO) and copy data over
    GLuint vbo; // uint descriptor to Vertex Buffer Object
    glGenBuffers(1, &vbo); // Generate 1 buffer

    GLfloat vertices[] = {
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,

        0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,

        -1.0f, -1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f,  1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        1.0f,  1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
    };
    glBindBuffer(GL_ARRAY_BUFFER, vbo); // makes VBO active array_buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // copy data

    // Add element buffer to control order of rengering
    GLuint ebo;
    glGenBuffers(1, &ebo);

    // Create/compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); // int descriptor to shader object
    glShaderSource(vertexShader, 1, &vertexSource, NULL); // makes active
    glCompileShader(vertexShader);

    // Check successful shader compilation
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
        printf("Shader creation failed with error log:\n%s\n", buffer);
        return EXIT_FAILURE;
    }

    // Create/compile fragmentshader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    // Check successful shader compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, buffer);
        printf("Shader creation failed with error log:\n%s\n", buffer);
        return EXIT_FAILURE;
    }


    // Link shaders into shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor"); // fragment shader allowed to write multiple buffers
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    // Specify the layout of the vertex data
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);

    GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
    glEnableVertexAttribArray(colAttrib);
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    // Specify texture
    GLuint textures[2];
    glGenTextures(2, textures);

    glActiveTexture(GL_TEXTURE0);
	textures[0] = loadTexture("sample.png");
    glUniform1i(glGetUniformLocation(shaderProgram, "texKitten"), 0);
    glActiveTexture(GL_TEXTURE1);
	textures[1] = loadTexture("sample2.png");
    glUniform1i(glGetUniformLocation(shaderProgram, "texPuppy"), 0);


    // Model matrix
    GLint uniModel = glGetUniformLocation(shaderProgram, "model");

    // View matrix
    glm::mat4 view = glm::lookAt(
            glm::vec3(1.6f, 1.6f, 1.6f), // position of the camera
            glm::vec3(0.0f, 0.0f, 0.0f), // point to be centered on-screen
            glm::vec3(0.0f, 0.0f, 1.0f)  // up axis (+z direction)
    );
    GLint uniView = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

    // Projection matrix
    glm::mat4 proj = glm::perspective(
            glm::radians(60.0f), // vertical field-of-view
            800.0f / 600.0f, // aspect ratio (width/height)
            1.0f, // near plane
            10.0f); // far plane
    GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));



    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
        // Wireframe mode
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

        // Calculate time
        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        // Update transform
        glm::mat4 model;
        model = glm::rotate(
                model,
                time * glm::radians(180.0f),
                glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

        // Background Fill Color
        glClearColor(1.0f, 1.0, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw cube scene
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glEnable(GL_STENCIL_TEST);

            // floor: disable depth buffer when drawing floor to prevent occluding reflection due to depth testing
            glStencilFunc(GL_ALWAYS, 1, 0xFF); // Set any stencil to 1
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glStencilMask(0xFF); // Write to stencil buffer
            glDepthMask(GL_FALSE); // Don't write to depth buffer
            glClear(GL_STENCIL_BUFFER_BIT); // Clear stencil buffer (0 by default)

            glDrawArrays(GL_TRIANGLES, 36, 6);

            // Draw cube reflection
            glStencilFunc(GL_EQUAL, 1, 0xFF); // Pass test if stencil value is 1
            glStencilMask(0x00); // Don't write anything to stencil buffer
            glDepthMask(GL_TRUE);


            model = glm::scale(
                    glm::translate(model, glm::vec3(0, 0, -1)),
                    glm::vec3(1, 1, -1)
                    );
            glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

            GLint uniColor = glGetUniformLocation(shaderProgram, "overrideColor");
            glUniform3f(uniColor, 0.3f, 0.3f, 0.3f);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            glUniform3f(uniColor, 1.0f, 1.0f, 1.0f);

        glDisable(GL_STENCIL_TEST);

        // Bind default framebuffer and draw contents of `frameBuffer`
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindVertexArray(vaoQuad);
        glDisable(GL_DEPTH_TEST);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texColorBuffer);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }   glfwTerminate();
    return EXIT_SUCCESS;
}
