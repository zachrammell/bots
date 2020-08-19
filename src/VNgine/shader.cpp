#include <VNgine/shader.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

#include <glm/gtc/type_ptr.hpp>

#include <VNgine/helper.h>

namespace fs = std::filesystem;

namespace VNgine
{

Shader Shader::CreateVS(std::string_view name, std::string_view source)
{
  return Shader{ name, Type::VERTEX , source.data() };
}
Shader Shader::CreateFS(std::string_view name, std::string_view source)
{
  return Shader{ name, Type::FRAGMENT , source.data() };
}

Shader::Shader(std::string_view name, Type type, char const* source)
  : name_{ name },
  type_{ type }
{
  assert(type != Type::INVALID);

  id_ = glCreateShader(to_integral(type));
  glShaderSource(id_, 1, &source, nullptr);
  glCompileShader(id_);

  int success;
  glGetShaderiv(id_, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    char info_log[512];
    glGetShaderInfoLog(id_, 512, nullptr, info_log);
    std::cerr << "[ERROR] Shader compilation failed: " <<
      name << ((type == Type::VERTEX) ? ".vs" : ".fs") << "\n" <<
      info_log << std::endl;
    assert(!"Shader compilation failed.");
  }
}

Shader::Type Shader::getType() const
{
  return type_;
}

std::string_view Shader::getName() const
{
  return name_;
}

GLuint Shader::getID() const
{
  return id_;
}

GLint ShaderProgram::getUniformLocation(std::string_view name) const
{
  return glGetUniformLocation(id_, name.data());
}

void ShaderProgram::setUniform(GLint location, glm::mat4 const& matrix) const
{
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

ShaderPool::ShaderPool(std::string_view directory)
{
  fs::path const shader_path{ directory };
  entries_.reserve(number_of_files_in_directory(shader_path));

  for (auto const& file : fs::directory_iterator{ shader_path })
  {
    if (!file.is_regular_file())
    {
      continue;
    }

    fs::path const ext = file.path().extension();
    std::string const filename = file.path().filename().replace_extension("").string();
    if (ext.compare(".vs") == 0 || ext.compare(".fs") == 0)
    {
      std::stringstream source_stream;
      source_stream << std::ifstream{ file }.rdbuf();

      if (ext.c_str()[1] == 'v') // it's a .vs file
      {
        entries_.emplace_back(Shader::CreateVS(filename, source_stream.str()));
      }
      else // must be a .fs file
      {
        entries_.emplace_back(Shader::CreateFS(filename, source_stream.str()));
      }
    }
  }
}

Shader const& ShaderPool::getVS(std::string_view name) const
{
  for (Shader const& shader : entries_)
  {
    if (shader.getType() == Shader::Type::VERTEX)
    {
      if (shader.getName() == name)
      {
        return shader;
      }
    }
  }
  std::cerr << "Requested vertex shader: " << name << " not present.\n";
  assert(!"Requested vertex shader not present.");
}

Shader const& ShaderPool::getFS(std::string_view name) const
{
  for (Shader const& shader : entries_)
  {
    if (shader.getType() == Shader::Type::FRAGMENT)
    {
      if (shader.getName() == name)
      {
        return shader;
      }
    }
  }
  std::cerr << "Requested fragment shader: " << name << " not present.\n";
  assert(!"Requested fragment shader not present.");
}


ShaderProgram::ShaderProgram(ShaderPool const& pool, std::string_view vs_name, std::string_view fs_name)
  : id_{ glCreateProgram() }
{
  glAttachShader(id_, pool.getVS(vs_name).getID());
  glAttachShader(id_, pool.getFS(fs_name).getID());
  glLinkProgram(id_);

  int success;
  glGetProgramiv(id_, GL_LINK_STATUS, &success);
  if (!success)
  {
    char info_log[512];
    glGetProgramInfoLog(id_, 512, nullptr, info_log);
    std::cerr << "[ERROR] Shader program [" << vs_name << ".vs, " << fs_name << ".fs] linking failed.\n" << info_log << std::endl;
    assert(!"Shader program linking failed.");
  }
}

ShaderProgram::~ShaderProgram()
{
  glDeleteProgram(id_);
}

void ShaderProgram::use() const
{
  glUseProgram(id_);
}

}
