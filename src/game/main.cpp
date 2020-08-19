#include <string>
#include <string_view>
#include <iostream>

#include <Windows.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <VNgine/engine.h>
#include <VNgine/shader.h>
#include <VNgine/input.h>

namespace
{

glm::vec4 clear_color = { 0.5f, 0.5f, 0.5f, 1.0f };

float vertices[] = {
     0.5f,  0.5f, 0.0f,  // top right
     0.5f, -0.5f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f,  // bottom left
    -0.5f,  0.5f, 0.0f   // top left 
};
unsigned int indices[] = {
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
};

glm::vec3 positions[] = {
  glm::vec3(0.0f,  0.0f,  0.0f),
  glm::vec3(2.0f,  5.0f, -15.0f),
  glm::vec3(-1.5f, -2.2f, -2.5f),
  glm::vec3(-3.8f, -2.0f, -12.3f),
  glm::vec3(2.4f, -0.4f, -3.5f),
  glm::vec3(-1.7f,  3.0f, -7.5f),
  glm::vec3(1.3f, -2.0f, -2.5f),
  glm::vec3(1.5f,  2.0f, -2.5f),
  glm::vec3(1.5f,  0.2f, -1.5f),
  glm::vec3(-1.3f,  1.0f, -1.5f)
};

}

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

int main()
{
  std::cout << "[INFO] Game started.\n";

  VNgine::Window window = { 1184, 666, "Game" };
  window.show();

  VNgine::Input input = { window };

  glEnable(GL_DEPTH_TEST);
  glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);

  positions[0] = { 0, 0, 0 };

  glm::mat4 view = glm::mat4(1.0f);
  view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

  glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

  VNgine::ShaderPool const shader_pool{ "data/shaders" };
  VNgine::ShaderProgram const basic_shader{ shader_pool, "basic", "basic" };

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  GLuint VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(0));
  glEnableVertexAttribArray(0);

  basic_shader.use();
  GLint uModel, uView, uProjection;
  uModel = basic_shader.getUniformLocation("model");
  uView = basic_shader.getUniformLocation("view");
  uProjection = basic_shader.getUniformLocation("projection");

  while (!(window.shouldClose()))
  {   
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float radius = 10.0f;
    float camX = sinf(glfwGetTime()) * radius;
    float camZ = cosf(glfwGetTime()) * radius;
    view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

    basic_shader.setUniform(uView, view);
    basic_shader.setUniform(uProjection, projection);

    glBindVertexArray(VAO);
    for (int i = 0; i < 10; ++i)
    {
      glm::mat4 local_model = glm::mat4(1.0f);
      local_model = glm::translate(local_model, positions[i]);
      float const angle = 20.0f * i;
      local_model = glm::rotate(local_model, glm::radians(angle), glm::vec3{ 1.0f, 0.3f, 0.5f });
      basic_shader.setUniform(uModel, local_model);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    window.present();
    window.poll();
  }

  return 0;
}
