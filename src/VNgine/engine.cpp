#include <VNgine/engine.h>

#include <cassert>
#include <iostream>
#include <sstream>
#include <fstream>

namespace
{

void GLAPIENTRY
messageCallback(GLenum source,
  GLenum type,
  GLuint id,
  GLenum severity,
  GLsizei length,
  const GLchar* message,
  const void* userParam)
{
  // temp hack
  //if (severity > GL_DEBUG_SEVERITY_MEDIUM)
  {
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
      (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
      type, severity, message);
  }
}

void errorCallback(int error, const char* description)
{
  fprintf(stderr, "[ERROR]: GLFW error %i:\n%s\n", error, description);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
  // make sure the viewport matches the new window dimensions; note that width and 
  // height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

}

namespace VNgine
{

Window::Window(int width, int height, std::string_view title)
{
  glfwSetErrorCallback(errorCallback);

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#ifdef _DEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
  glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

  window_ = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
  glfwMakeContextCurrent(window_);
  glfwSwapInterval(1);

  glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
  glfwSetWindowAspectRatio(window_, 16, 9);

  GLenum err = glewInit();
  if (GLEW_OK != err)
  {
    /* Problem: glewInit failed, something is seriously wrong. */
    std::cerr << glewGetErrorString(err) << std::endl;
    assert(!"GLEW failed to initialize");
  }

  if (GLEW_KHR_debug)
  {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(messageCallback, nullptr);
  }
}
Window::~Window()
{
  glfwDestroyWindow(window_);
}
void Window::show() const
{
  glfwShowWindow(window_);
}
int Window::shouldClose() const
{
  return glfwWindowShouldClose(window_);
}
void Window::poll() const
{
  glfwPollEvents();
}
void Window::present() const
{
  glfwSwapBuffers(window_);
}
GLFWwindow* Window::getHandle() const
{
  return window_;
}

}
