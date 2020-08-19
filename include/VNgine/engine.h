#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <VNgine/helper.h>
#include <VNgine/thunk.h>

namespace VNgine
{

class Window : non_copyable<Window>
{
public:
  Window(int width, int height, std::string_view title);
  ~Window();
  void show() const;
  int shouldClose() const;
  void poll() const;
  void present() const;

  GLFWwindow* getHandle() const;
private:
  GLFWwindow* window_;
};

}
