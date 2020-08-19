#include <VNgine/input.h>

#include <iostream>

namespace VNgine
{

void g_keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  std::cout << "Key: " << (char)key << "\n";
}

Input::Input(Window const& window)
  : member_{ 0x69 },
    key_callback_thunk_{ this, &Input::keyCallback }
{
  // The thunk callback can be called from a C library like GLFW
  glfwSetKeyCallback(window.getHandle(), key_callback_thunk_.getCallback());
}


  /* Press G without pushing any other buttons. This should print:

Window: 0x000002CACB024AA0 [Not this exact pointer, some pointer]
Key: G
Scancode: 0x22
Action: 1
Mods: 0
Member is: 105

   */
void Input::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  std::cout << "Window: 0x" << window << "\n";
  std::cout << "Key: " << (char)key << "\n";
  std::cout << "Scancode: 0x" << std::hex << scancode << std::dec << "\n";
  std::cout << "Action: " << action << "\n";
  std::cout << "Mods: " << mods << "\n";
  std::cout << "Member is: " << member_ << "\n";
}

}