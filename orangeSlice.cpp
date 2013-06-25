//
//  Display a sherical wedge (orange slice)
//    wikipedia: http://en.wikipedia.org/wiki/Spherical_wedge
//
//  - Drag with the mouse to rotate the wedge
//
//                        Lazaros Karydas      01.30.2013

#include "Angel.h"
#include <iostream>
#include <assert.h>
#include <vector>

using namespace std;
typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

// Mouse wheel doesn't do anything
#if !defined(GLUT_WHEEL_UP)
#define GLUT_WHEEL_UP 3
#define GLUT_WHEEL_DOWN 4
#endif

#define SCALE_VECTOR 0.5
float zoom_z = 10;


GLuint vPosition;
GLuint vNormal;

int winHeight = 480;
int winWidth = 640;

bool mouseDown = false;

float xrot = 0;
float yrot = 0;

float xdiff = 0;
float ydiff = 0;
GLfloat step = 5;
// Camera coordinates
vec4 u = vec4(1, 0, 0, 0);
vec4 v = vec4(0, 1, 0, 0);
vec4 n = vec4(0, 0, 1, 0);
// Eye direction
vec4 eye = vec4(0,0,5,1);

// The vertices of the geometry
point4* vertices;

// Index to points
int triangleCount = 0;

// Stores the points and normals to load into the VB
point4* points;
vec4*   normals;

// Index to points
int pointIndex = 0;

void triangle( int a, int b, int c, bool useVertexAsNormal);
void quad( int a, int b, int c, int d, bool useVertexAsNormal);

/*
   Creates the geometry of a spherical wedge (orange slice).
   angle: Angle of the wedge, in radians
   theta_step, phi_step: Step in radians that controls the quality of the geometry across theta and phi
   radius: The radius of the wedge
*/
void createWedge(float angle, float theta_step, float phi_step, int radius)
{
  // Constants
  const float pi = 3.1415926535;
  const float errorMargin = 0.00000001; // Takes care of precision errors in floating-point arithmetic

  // Cartesian coordinates
  float x, y, z;

  // Count the number of points across the phi direction
  int p = 0;
  for (float phi = 0; phi <= angle + errorMargin; phi += phi_step) 
    p++;
  cout << "p: " << p << endl;

  // Count the number of points across the half-circles curved edge (theta direction)
  int q = 0;
  for (float theta = 0 + theta_step; theta < pi; theta += theta_step) 
    q++;
  q += 2; // Add the theta == 0 and theta == pi points
  cout << "q: " << q << endl;

  // Calculate the number of vertices
  // curved surface needs (q - 2) * p + 2, plus one for the coordinate system center
  int nofVertices = (q - 2) * p + 3;
  cout << "nofVertices: " << nofVertices << endl;

  // Allocate memory
  vertices = new point4[nofVertices];

  // Useful indexes
  int indSouth = nofVertices - 2;
  int indCenter = nofVertices - 1;

  // _______________________________________________ Generate the triangles
  int index = 0;
  GLfloat theta = 0;
  GLfloat phi = 0;
  // Generate the north pole vertex
  x = radius * sin(theta) * cos(phi);
  y = radius * sin(theta) * sin(phi);
  z = radius * cos(theta);
  vertices[index] = point4(x, y, z, 1.0);
  index++;
  // Generate the curved surface vertices
  for (theta = 0 + theta_step; theta < pi; theta += theta_step)
    for (phi = 0; phi <= angle+errorMargin; phi += phi_step)
      {
	x = radius * sin(theta) * cos(phi);
	y = radius * sin(theta) * sin(phi);
	z = radius * cos(theta);
	vertices[index] = point4(x, y, z, 1.0);
	index++;
      }
  // Generate the south pole vertex
  theta = pi;
  x = radius * sin(theta) * cos(phi);
  y = radius * sin(theta) * sin(phi);
  z = radius * cos(theta);
  vertices[index] = point4(x, y, z, 1.0);
  index++;
  // Add the (0,0,0) vertex
  vertices[index] = point4(0, 0, 0, 1.0);

  // Calculate the number of points to push down the pipeline
  int nofPoints = ( 2*(p - 1) + 2*(p - 1) * (q - 3) ) * 3 + (q - 1)*3 * 2;
  cout << "nofPoints : " << nofPoints << endl;
  // Allocate memory for points and normals
  points = new point4[nofPoints]; 
  normals = new vec4[nofPoints];

  // _______________________________________________ Generate the triangles
  int a, b, c, d;
  // Triangles only near the north pole
  for (int j = 1; j < p; j++)
    {
      a = 0;
      b = j;
      c = j + 1;
      triangle(a, b, c, true);
    }
  // Quads (triangle pairs) around the curved surface
  for (int i = 0; i < q - 3; i++)      
    for (int j = 1; j < p; j++)
      {
	a = i * p + j;
	b = i * p + p + j;
	c = i * p + p + j + 1;
	d = i * p + j + 1;
	quad(a, b, c, d, true);
      }
  // Triangles only near the south pole
  for (int j = 1; j < p; j++)
    {
      a = indSouth - j;
      b = indSouth - 1 - j;
      c = indSouth;
      triangle(a, b, c, true);
    }
  // Triangles for the sides
  triangle(1, 0, indCenter, false);
  triangle(indCenter, 0, p, false);
  triangle(indCenter, (q -2)*p, indSouth, false);
  for (int j = 0; j < q - 2; j++)
    {
      a = indCenter;
      b = (j + 1)*p + 1;
      c = j * p + 1;
      triangle(a, b, c, false);
    }
  for (int j = 0; j < q - 3; j++)
    {
      a = indCenter;
      b = (j + 1) * p;
      c = (j + 2) * p;
      triangle(a, b, c, false);
    }

  cout << "pointIndex: " << pointIndex << endl;
}

void quad( int a, int b, int c, int d, bool useVertexAsNormal)
{
  triangle(a, b, c, useVertexAsNormal);
  triangle(a, c, d, useVertexAsNormal);
}
void triangle( int a, int b, int c, bool useVertexAsNormal)
{
  vec4 normal;
  if (useVertexAsNormal)
    {
      normal = vertices[a];
      normal.w = 0;
    }
  else 
    {
      // Compute a normal for this face (all 3 edges will be assigned the same normal)
      vec4 u = vertices[b] - vertices[a];
      vec4 v = vertices[c] - vertices[b];
      normal = normalize( cross(u, v) );
    }
  normals[pointIndex] = normalize(normal); points[pointIndex] = vertices[a]; pointIndex++;
  if (useVertexAsNormal)
    {
      normal = vertices[b]; 
      normal.w = 0;
    }
  normals[pointIndex] = normalize(normal); points[pointIndex] = vertices[b]; pointIndex++;
  if (useVertexAsNormal)
    {
      normal = vertices[c];
      normal.w = 0;
    }
  normals[pointIndex] = normalize(normal); points[pointIndex] = vertices[c]; pointIndex++;
}

// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Xaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };

// Model-view and projection matrices uniform location
GLuint  ModelViewCam, ModelViewObj, Projection, LightPosition;
//point4 light_position_distant;

// Get OpenGL version information
void getGLinfo()
{
  cout << "GL Vendor   : " << glGetString(GL_VENDOR) << endl;
  cout << "GL Renderer : " << glGetString(GL_RENDERER) << endl;
  cout << "GL Version  : " << glGetString(GL_VERSION) << endl;
}

void mouseMotionCB(int x, int y)
{
  if (mouseDown)
    {
      yrot = x - xdiff;
      xrot = y + ydiff;
      glutPostRedisplay();
    }
}

void mouseCB(int button, int state, int x, int y)
{
  switch(button){
  case GLUT_LEFT_BUTTON:
    switch(state)
      {
      case GLUT_DOWN:
	mouseDown = true;
	xdiff = x - yrot;
	ydiff = -y + xrot;
	break;
      case GLUT_UP:
	mouseDown = false;
	break;
      default:
	break;
      }
    break;    
  case GLUT_MIDDLE_BUTTON:
    break;
  case GLUT_WHEEL_UP:
    zoom_z -= 0.2;
    glutPostRedisplay();
    break;
  case GLUT_WHEEL_DOWN:
    zoom_z += 0.2;
    glutPostRedisplay();
    break;
  default:
    break;
  }
}


//----------------------------------------------------------------------------

// OpenGL initialization

void init()
{
  getGLinfo();
  createWedge(0.785, 0.02, 0.02, 2);

  // Load shaders and use the resulting shader program
  GLuint program = InitShader( "shader_vert.glsl", "shader_frag.glsl" );
  glUseProgram( program );

  // Create and bind a vertex array object
  GLuint vao;
  glGenVertexArrays( 1, &vao );
  glBindVertexArray( vao );

  // Create and bind a buffer object
  GLuint buffer;
  glGenBuffers( 1, &buffer );
  glBindBuffer( GL_ARRAY_BUFFER, buffer );
  GLsizeiptr bufferSize = pointIndex * sizeof(GLfloat) * 4; // calculate the buffer size
  cout << "bufferSize: " << bufferSize << endl;
  // Copy data
  glBufferData( GL_ARRAY_BUFFER, 2 * bufferSize, NULL, GL_STATIC_DRAW );
  // Load points and normals to video device memory
  glBufferSubData( GL_ARRAY_BUFFER, 0, bufferSize, points );
  glBufferSubData( GL_ARRAY_BUFFER, bufferSize,  bufferSize, normals );

  // Release points and normals from main memory; we don't need them any more
  delete[] points;
  delete[] normals;
  // If we are not going to make any more meshes with the original vertices, 
  // then releash them too
  delete[] vertices;

  // Bind with shader inputs
  vPosition = glGetAttribLocation( program, "in_vertex" );
  glEnableVertexAttribArray( vPosition );
  glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

  vNormal = glGetAttribLocation( program, "in_normal" ); 
  glEnableVertexAttribArray( vNormal );
  glVertexAttribPointer( vNormal, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( bufferSize ));
 
  // _________________________________________________ Light material
  color4 light_ambient( 1, 1, 1, 1.0 );
  color4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
  color4 light_specular( 1.0, 1.0, 1.0, 1.0 );
  // _________________________________________________ Ambient material
  color4 material_ambient( 0.1, 0.055, 0.0, 1.0 );
  color4 material_diffuse( 1.0, 0.55, 0.0, 1.0 );
  color4 material_specular( 1.0, 0.55, 0.0, 1.0 );
  float  material_shininess = 30.0;
  // _________________________________________________ Ambient product
  color4 ambient_product = light_ambient * material_ambient;
  color4 diffuse_product = light_diffuse * material_diffuse;
  color4 specular_product = light_specular * material_specular;

  // CURRENT LIGHT POSITION
  //POSITION
  vec4 lightPosition = vec4(0.0, 10.0, 10.0, 1.0);
  // DIRECTION
  vec4 lightDirection = vec4(0, -1, 0.0, 0);
  vec4 ldc = vec4(0,10,0,0);
  float spotCosCutoff = 0.6;

  glUniform4fv( glGetUniformLocation(program, "mat_ambient"),1, ambient_product );
  glUniform4fv( glGetUniformLocation(program, "mat_diffuse"),1, diffuse_product );
  glUniform4fv( glGetUniformLocation(program, "mat_specular"),1, specular_product );
  glUniform1f( glGetUniformLocation(program, "mat_shininess"),material_shininess );
  glUniform4fv( glGetUniformLocation(program, "lightPosition" ), 1, lightPosition );
  glUniform4fv( glGetUniformLocation(program, "lightDirection"), 1,lightDirection );
  glUniform1f( glGetUniformLocation(program, "spotCosCutoff"), spotCosCutoff );

  // Retrieve transformation uniform variable locations
  ModelViewCam = glGetUniformLocation( program, "un_ModelViewMatrix" );
  ModelViewObj = glGetUniformLocation(program, "ModelViewObj");
  Projection = glGetUniformLocation( program, "un_ProjectionMatrix" );

  glEnable( GL_DEPTH_TEST );
  glClearColor( 0.0, 0.0, 0.0, 1.0 ); 
}

//----------------------------------------------------------------------------

void
display( void )
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  mat4 model_view = Angel::identity();
  model_view = model_view * LookAt(eye, eye-n, v);
  glUniformMatrix4fv(ModelViewCam, 1, GL_TRUE, model_view);
  model_view = Angel::identity();
  model_view = Scale(0.7, 0.7, 0.7) * Translate(0,0,0)*RotateX( xrot )*RotateY( yrot );
  // @TODO: Properly compute the normal matrix and pass it to the shader
  //        (inverse transpose of the upper 3x3 matrix (the rotation matrix) of the modelView matrix)
  // mat4 normal_matrix
  glUniformMatrix4fv( ModelViewObj, 1, GL_TRUE, model_view );
  glDrawArrays( GL_TRIANGLES, 0, pointIndex );
  glutSwapBuffers();
}

//----------------------------------------------------------------------------

void idle( void )
{
  //Theta[Axis] += 0.01;

  //if ( Theta[Axis] > 360.0 ) {
  //Theta[Axis] -= 360.0;
  //}
    
  //glutPostRedisplay();
}

void Timer(int extra)
{
  Theta[Axis] += 0.1;
  if ( Theta[Axis] > 360.0 ) {
    Theta[Axis] -= 360.0;
  }
    
  glutPostRedisplay();
  glutTimerFunc(10,Timer,0);
}

//----------------------------------------------------------------------------

void keyboard( unsigned char key, int x, int y )
{
  switch( key ) {
  case 'X':
    cout << "pitch up" << endl;
    v = normalize(cos(DegreesToRadians*step)*v 
		  - sin(DegreesToRadians*step)*n);
    n = normalize(sin(DegreesToRadians*step)*v 
		  + cos(DegreesToRadians*step)*n);
    break;
  case 'x':
    cout << "pitch down" << endl;
    v = normalize(cos(DegreesToRadians*step)*v 
		  + sin(DegreesToRadians*step)*n);
    n = normalize(-sin(DegreesToRadians*step)*v 
		  + cos(DegreesToRadians*step)*n);
    break;
  case 'C':
    cout << "Yaw counterclockwise" << endl;
    u = normalize(cos(DegreesToRadians*step)*u 
		  - sin(DegreesToRadians*step)*n);
    n = normalize(sin(DegreesToRadians*step)*u 
		  + cos(DegreesToRadians*step)*n);
    cout << "step =" << step << endl;
    cout << "u = " << u << endl;
    cout << "n = " << n << endl;
    break;
  case 'c':
    cout << "Yaw clockwise" << endl;
    u = normalize(cos(DegreesToRadians*step)*u 
		  + sin(DegreesToRadians*step)*n);
    n = normalize(-sin(DegreesToRadians*step)*u 
		  + cos(DegreesToRadians*step)*n);
    break;
  case 'Z':
    cout << "Roll counterclockwise" << endl;
    u = normalize(cos(DegreesToRadians*step)*u 
		  - sin(DegreesToRadians*step)*v);
    v = normalize(sin(DegreesToRadians*step)*u 
		  + cos(DegreesToRadians*step)*v);
    break;
  case 'z':
    cout << "Roll clockwise" << endl;
    u = normalize(cos(DegreesToRadians*step)*u 
		  + sin(DegreesToRadians*step)*v);
    v = normalize(-sin(DegreesToRadians*step)*u 
		  + cos(DegreesToRadians*step)*v);
    break;
  case 033: // Escape Key
  case 'q': case 'Q':
    exit( EXIT_SUCCESS );
    break;
  }
  glutPostRedisplay();
}

//----------------------------------------------------------------------------

void reshape( int width, int height )
{
  //cout << "reshape" << endl;
  //vec4 light_position_distant = vec4(0.0, 100.0, 1.0, 0.0);//Angel::identity();
  //  light_position_distant = 
  //cout << light_position_distant << endl;
  
  glViewport( 0, 0, width, height );

  GLfloat aspect = GLfloat(width)/height;
  mat4  projection = Perspective( 45.0, aspect, 0.0001, 300.0 );

  glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );
  //glUniformMatrix4fv(LightPosition, 1, GL_TRUE, light_position_distant);
}

void specialKeys(int key, int x, int y)
{
  //cout << "--keyboard2--" << endl;
  switch( key ) {

  case GLUT_KEY_UP: // MOVE FORWARD
    eye[0] -= SCALE_VECTOR * n[0];
    eye[1] -= SCALE_VECTOR * n[1];
    eye[2] -= SCALE_VECTOR * n[2];
    break;
  case GLUT_KEY_DOWN: // MOVE Backward
    eye[0] += SCALE_VECTOR * n[0];
    eye[1] += SCALE_VECTOR * n[1];
    eye[2] += SCALE_VECTOR * n[2];
    break;
  case GLUT_KEY_RIGHT: // MOVE right
    eye[0] += SCALE_VECTOR * u[0];
    eye[1] += SCALE_VECTOR * u[1];
    eye[2] += SCALE_VECTOR * u[2];
    break;
  case GLUT_KEY_LEFT: // MOVE left
    eye[0] -= SCALE_VECTOR * u[0];
    eye[1] -= SCALE_VECTOR * u[1];
    eye[2] -= SCALE_VECTOR * u[2];
    break;
  default:
    break;
  }
  glutPostRedisplay();
}

//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
  glutInitWindowSize( 1280, 1024 );
  //glutInitContextVersion( 3, 2 );
  //glutInitContextProfile( GLUT_CORE_PROFILE );
  glutCreateWindow( "An orange slice" );
  glewInit();
  init();
  glutDisplayFunc( display );
  glutKeyboardFunc( keyboard );
  glutSpecialFunc (specialKeys);
  glutReshapeFunc( reshape );
  //glutTimerFunc(10, Timer, 0);
  glutMouseFunc(mouseCB);
  glutMotionFunc(mouseMotionCB);
  glutMainLoop();
  return 0;
}
