#pragma once
#include <VNgine/engine.h>

namespace VNgine
{

class Input
{
public:
  Input(Window const& window);

  // Simple on-keypress callback to test whether it can access params and `this` ptr
  void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
  long long member_;
  Thunk<Input, GLFWkeyfun> key_callback_thunk_;
};

}
