#version 120
#extension GL_EXT_geometry_shader4 : enable
 
void main() {
  for(int i = 0; i < gl_VerticesIn; ++i) {
    gl_FrontColor = gl_FrontColorIn[i];
    gl_Position = gl_PositionIn[i];
	gl_TexCoord[0] = gl_MultiTexCoordIn[i][0];
    EmitVertex();
  }
}