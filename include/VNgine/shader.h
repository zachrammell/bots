#pragma once

#include <string_view>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

namespace VNgine
{

class Shader
{
public:
  enum class Type : int
  {
    INVALID,
    VERTEX = GL_VERTEX_SHADER,
    FRAGMENT = GL_FRAGMENT_SHADER
  };

  static Shader CreateVS(std::string_view name, std::string_view source);
  static Shader CreateFS(std::string_view name, std::string_view source);

  std::string_view getName() const;
  Type getType() const;
  GLuint getID() const;

private:
  std::string name_;
  Type type_ = Type::INVALID;
  GLuint id_ = std::numeric_limits<GLuint>::max();

  Shader(std::string_view name, Type type, char const* source);
};

class ShaderPool
{
public:
  ShaderPool(std::string_view directory);
  Shader const& getVS(std::string_view name) const;
  Shader const& getFS(std::string_view name) const;
private:
  std::vector<Shader> entries_;
};

class ShaderProgram
{
public:
  ShaderProgram(ShaderPool const& pool, std::string_view vs_name, std::string_view fs_name);
  ~ShaderProgram();
  void use() const;
  GLint getUniformLocation(std::string_view name) const;
  void setUniform(GLint location, glm::mat4 const& matrix) const;
private:
  GLuint id_;
};

}