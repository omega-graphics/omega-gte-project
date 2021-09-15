struct VertexRaster {
  vec4 pos;
};

void main(){
  uint pos = gl_VertexIndex;

  VertexRaster st;
  gl_Position = st;
}
