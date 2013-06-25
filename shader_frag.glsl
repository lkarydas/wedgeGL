#version 150 

// per-fragment interpolated values from the vertex shader
smooth in vec3 fN;
smooth in vec3 fL;
smooth in vec3 fE;

out vec4 out_fragColor;
in vec3 D;


uniform vec4 mat_ambient;
uniform vec4 mat_diffuse;
uniform vec4 mat_specular;
uniform float mat_shininess;

uniform float spotCosCutoff;

void main() 
{
  vec4 AmbientProduct = mat_ambient;
  vec4 DiffuseProduct = mat_diffuse;
  vec4 SpecularProduct = mat_specular;
  
  // Normalize the input lighting vectors
  vec3 L = normalize(fL);
  vec3 E = normalize(fE);
  vec3 N = normalize(fN);


  vec3 H = normalize( L + E );
        
  vec4 ambient = AmbientProduct;
  out_fragColor = ambient;

  float angle = dot(-L,D);

  if (angle > spotCosCutoff) 
    {

      float Kd = max(dot(L, N), 0.0);
      vec4 diffuse = Kd*DiffuseProduct;
    
      float Ks = pow(max(dot(N, H), 0.0), mat_shininess);
      vec4 specular = Ks*SpecularProduct;

      // discard the specular highlight if the light's behind the vertex
      if( dot(L, N) < 0.0 )
	{
	  specular = vec4(0.0, 0.0, 0.0, 1.0);
	}
      out_fragColor += angle*(diffuse + specular);

    }


  out_fragColor.a = mat_diffuse.a;


} 
