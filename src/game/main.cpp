#include <iostream>
#include <Windows.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace
{
  glm::vec4 clear_color = { 1.0f, 0.75f, 0.33f, 1.0f };
}

int main()
{
  std::cout << "Game started.\n";

  glfwInit();
  GLFWwindow* window = glfwCreateWindow(800, 400, "Robogirls Game", nullptr, nullptr);
  glfwShowWindow(window);

  glfwMakeContextCurrent(window);

  glfwSwapInterval(1);

  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();

    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);

  return 0;
}
