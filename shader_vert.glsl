#version 150

in vec4 in_vertex;
in vec4 in_normal;

// output values that will be interpretated per-fragment
smooth out vec3 fN;
smooth out vec3 fE;
smooth out vec3 fL;
out vec3 D;

uniform mat4 un_ModelViewMatrix;
// @TODO: Pass normal_Matrix to the shader (inverse transpose of modelview)
uniform mat4 normal_Matrix;
uniform mat4 ModelViewObj;

uniform vec4 lightPosition;
uniform vec4 lightDirection;


uniform mat4 un_ProjectionMatrix;

void main()
{


  vec3 pos = (un_ModelViewMatrix*ModelViewObj*in_vertex).xyz;  

  fN = (un_ModelViewMatrix*ModelViewObj*vec4(in_normal.x, in_normal.y, in_normal.z, 0.0)).xyz;
  


  if (lightPosition.w != 0)
    fL = (un_ModelViewMatrix*lightPosition).xyz - pos;
  else
    fL = -(un_ModelViewMatrix*lightPosition).xyz;

  fE =-pos;

  D = normalize((un_ModelViewMatrix*lightDirection).xyz);
 

  gl_Position = un_ProjectionMatrix*un_ModelViewMatrix*ModelViewObj*in_vertex;
  //gl_Position = in_vertex;
}
