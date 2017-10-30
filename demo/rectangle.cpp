#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <freijo/VAO.hpp>
#include <freijo/buffer.hpp>
#include <freijo/program.hpp>
#include <freijo/shader.hpp>
#include <glm/vec3.hpp>

#include <iostream>

const std::string vtxSrc = R"(
#version 330 core
layout (location = 0) in vec3 pos;

void main()
{
  gl_Position = vec4(pos, 1.0);
}
)";

const std::string fragSrc = R"(
#version 330 core
out vec4 color;

void main()
{
  color = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
)";

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{ glViewport(0, 0, width, height); }

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL",
                                          NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    {
        auto program = freijo::program{
            freijo::vertex_shader(vtxSrc).id(),
            freijo::fragment_shader(fragSrc).id(),
        };
        freijo::VBO<glm::vec3> vertices{
            { 0.5,  0.5, 0},
            { 0.5, -0.5, 0},
            {-0.5, -0.5, 0},
            {-0.5,  0.5, 0},
        };
        
        freijo::EBO<unsigned int> idxs{
            0, 1, 3,
            1, 2, 3
        };
        
        freijo::VAO vao;
        vao.attach(0, vertices);
        vao.attach(idxs);
        
        while(!glfwWindowShouldClose(window))
        {
            processInput(window);
        
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            program.use();
            freijo::scoped_vao_bind s(vao);
            glDrawElements(GL_TRIANGLES, idxs.size(),
                           decltype(idxs)::target::GLtype, 0);

            glfwSwapBuffers(window);
            glfwPollEvents();    
        }
    }
    glfwTerminate();
}
