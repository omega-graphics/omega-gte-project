// Warning! This file has been generated by omegaslc! DO NOT EDIT!!
#version 450
struct MyVertex2 {
  vec4 pos;
  vec2 coord;
};

layout(location =1) out vec2 VertexRaster2_coord;
layout(std430,set = 0,binding = 0) buffer v_buffer_2_Layout
{
MyVertex2 v_buffer_2[];
};
void main(){
  uint v_id = gl_VertexIndex;

  MyVertex2 v = v_buffer_2[v_id];
  ;
  gl_Position = v.pos;
  VertexRaster2_coord = v.coord;
  }