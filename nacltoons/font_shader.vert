uniform mat4 a_MVP;
attribute vec2 a_texCoord;
attribute vec4 a_position;
varying  vec2 v_texCoord;


void main()
{
  gl_Position = a_MVP * a_position;
  v_texCoord = a_texCoord;
}