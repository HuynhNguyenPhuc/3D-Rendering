#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

std::string loadShaderSource(const char* filePath) {
    std::ifstream shaderFile;
    shaderFile.open(filePath);
    
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    
    shaderFile.close();
    
    return shaderStream.str();
}

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    return shader;
}

GLuint createShaderProgram() {
    std::string vertexCode = loadShaderSource("path/to/vertex_shader.glsl");
    std::string fragmentCode = loadShaderSource("path/to/fragment_shader.glsl");

    const char* vertexShaderSource = vertexCode.c_str();
    const char* fragmentShaderSource = fragmentCode.c_str();

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

class SolidSphere {
    private:
        std::vector<GLfloat> vertices;
        std::vector<GLfloat> normals;
        std::vector<GLuint> indices;
        GLuint vao, vbo, nbo, ebo;

    public:
        SolidSphere(float radius, unsigned int rings, unsigned int sectors) {
            const float R = 1.0f / (float)(rings - 1);
            const float S = 1.0f / (float)(sectors - 1);
            vertices.resize(rings * sectors * 3);
            normals.resize(rings * sectors * 3);
            indices.resize((rings - 1) * (sectors - 1) * 6);

            std::vector<GLfloat>::iterator v = vertices.begin();
            std::vector<GLfloat>::iterator n = normals.begin();
            for (unsigned int r = 0; r < rings; ++r) {
                for (unsigned int s = 0; s < sectors; ++s) {
                    const float y = sin(-M_PI_2 + M_PI * r * R);
                    const float x = cos(2 * M_PI * s * S) * sin(M_PI * r * R);
                    const float z = sin(2 * M_PI * s * S) * sin(M_PI * r * R);

                    *v++ = x * radius;
                    *v++ = y * radius;
                    *v++ = z * radius;

                    *n++ = x;
                    *n++ = y;
                    *n++ = z;
                }
            }

            std::vector<GLuint>::iterator i = indices.begin();
            for (unsigned int r = 0; r < rings - 1; ++r) {
                for (unsigned int s = 0; s < sectors - 1; ++s) {
                    GLuint first = r * sectors + s;
                    GLuint second = r * sectors + (s + 1);
                    GLuint third = (r + 1) * sectors + (s + 1);
                    GLuint fourth = (r + 1) * sectors + s;

                    *i++ = first;
                    *i++ = second;
                    *i++ = third;
                    *i++ = first;
                    *i++ = third;
                    *i++ = fourth;
                }
            }

            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
            glGenBuffers(1, &nbo);
            glGenBuffers(1, &ebo);

            glBindVertexArray(vao);

            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(0);

            glBindBuffer(GL_ARRAY_BUFFER, nbo);
            glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), &normals[0], GL_STATIC_DRAW);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(1);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

            glBindVertexArray(0);
        }

        ~SolidSphere() {
            glDeleteBuffers(1, &vbo);
            glDeleteBuffers(1, &nbo);
            glDeleteBuffers(1, &ebo);
            glDeleteVertexArrays(1, &vao);
        }

        void draw(GLuint shaderProgram) {
            glUseProgram(shaderProgram);
            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
};

bool initOpenGL(GLFWwindow** window) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    *window = glfwCreateWindow(800, 600, "OpenGL Sphere", nullptr, nullptr);
    if (!*window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(*window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    return true;
}

int main() {
    GLFWwindow* window;
    if (!initOpenGL(&window)) return -1;

    GLuint shaderProgram = createShaderProgram();
    SolidSphere sphere(2.0f, 36, 18);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -5.0f));

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -7.0f));

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(shaderProgram, "light_pos"), 4.0f, 4.0f, 4.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "view_pos"), -1.0f, -1.0f, -1.0f);

        sphere.draw(shaderProgram);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}