#version 330 core
out vec4 frag_color;

in vec3 frag_pos;

void main()
{
  frag_color = vec4(frag_pos * 0.25f, 1.0f);
} 
