precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D s_texture;
uniform vec3 s_color;

void main()
{
  // Use the texture as pure ALPHA information, and then use the preset color
  gl_FragColor = texture2D( s_texture, vec2(v_texCoord.x,1.0 - v_texCoord.y) ) + vec4(s_color.x,s_color.y,s_color.z,0);
}