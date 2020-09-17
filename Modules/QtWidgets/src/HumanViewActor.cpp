#include "HumanViewActor.h"

#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>

#include "HumanModel.h"

vtkStandardNewMacro(HumanViewActor);

const GLchar* VERTEX_SHADER =
"#version 150\n"

"#define COLOR (vec3(.7))\n"
"#define LIGHT_POS (vec3(0.,0.,-5.))\n"

"uniform mat4 in_rot;"
"in vec3 in_vertex;"
"in vec3 in_normal;"
"out vec3 vec_color;"
"void main()"
"{"
  "gl_Position = in_rot * vec4(in_vertex, .5);"
  "vec3 rot_normal = (in_rot * vec4(in_normal, 1.)).xyz;"
  "vec3 light = normalize(LIGHT_POS - in_vertex);"
  "vec_color = COLOR * max(.2,dot(rot_normal, light));"
"}";

const GLchar* FRAGMENT_SHADER =
"#version 150\n"
"in vec3 vec_color;"
"out vec4 frag_color;"
"void main()"
"{"
  "frag_color = vec4(vec_color, 1.);"
"}";

void compileShader(GLuint shaderId, const GLchar* source)
{
  glShaderSource(shaderId, 1, &source, nullptr);
  glCompileShader(shaderId);

  GLint status = GL_FALSE;
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    int infoLogLength;
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
    char* errorMessage = new char[infoLogLength + 1];
    glGetShaderInfoLog(shaderId, infoLogLength, nullptr, errorMessage);
    printf("HumanViewActor Shader Error: %s\n", errorMessage);
  }
}

void linkProgram(GLuint program, GLuint vertexShader, GLuint fragmentShader)
{
  glLinkProgram(program);

  GLint status = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    int infoLogLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    char* errorMessage = new char[infoLogLength + 1];
    glGetProgramInfoLog(program, infoLogLength, nullptr, errorMessage);
    printf("HumanViewActor Program Error: %s\n", errorMessage);
  }

  glDetachShader(program, vertexShader);
  glDetachShader(program, fragmentShader);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

HumanViewActor::HumanViewActor()
{
}

void HumanViewActor::AllocateBuffers()
{
  // Alloc human buffers
  glGenVertexArrays(1, &m_Vao);
  glBindVertexArray(m_Vao);

  glGenBuffers(2, m_Vbo);

  glBindBuffer(GL_ARRAY_BUFFER, m_Vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(lowHumanVertices), lowHumanVertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, m_Vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(lowHumanNormals), lowHumanNormals, GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glGenBuffers(1, &m_Ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(lowHumanIndices), lowHumanIndices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  compileShader(vertexShader, VERTEX_SHADER);
  compileShader(fragmentShader, FRAGMENT_SHADER);

  m_Program = glCreateProgram();
  glAttachShader(m_Program, vertexShader);
  glAttachShader(m_Program, fragmentShader);
  linkProgram(m_Program, vertexShader, fragmentShader);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  m_ProgramRot = glGetUniformLocation(m_Program, "in_rot");
}

HumanViewActor::~HumanViewActor()
{
  if (!m_FirstRender && GLEW_VERSION_3_0) {
    glDeleteBuffers(2, m_Vbo);
    glDeleteBuffers(1, &m_Ibo);
    glDeleteVertexArrays(1, &m_Vao);
    glDeleteProgram(m_Program);
  }
}

// No library that implements this with sane API. Too bad!
void LookAt(const float center[3], const float up[3], GLfloat mat[16])
{
  float x[3], y[3], z[3];

  z[0] = -center[0];
  z[1] = -center[1];
  z[2] = -center[2];
  vtkMath::Normalize(z);

  y[0] = up[0];
  y[1] = up[1];
  y[2] = up[2];

  vtkMath::Cross(y, z, x);
  vtkMath::Cross(z, x, y);

  vtkMath::Normalize(x);
  vtkMath::Normalize(y);

  mat[0] = x[0];
  mat[4] = x[1];
  mat[8] = x[2];
  mat[12] = 0.f;
  mat[1] = y[0];
  mat[5] = y[1];
  mat[9] = y[2];
  mat[13] = 0.f;
  mat[2] = z[0];
  mat[6] = z[1];
  mat[10] = z[2];
  mat[14] = 0.f;
  mat[3] = 0.f;
  mat[7] = 0.f;
  mat[11] = 0.f;
  mat[15] = 1.f;
}

int HumanViewActor::RenderOverlay(vtkViewport* viewport)
{
  if (m_FirstRender) {
    // God forbids using glew before window is created
    if (!GLEW_VERSION_3_0) {
      return 0;
    }
    AllocateBuffers();
    m_FirstRender = false;
  }

  double* camP = mainCamera->GetPosition();
  double* camFP = mainCamera->GetFocalPoint();
  double* camUp = mainCamera->GetViewUp();

  float point[3] = { camFP[0] - camP[0], camFP[1] - camP[1], camFP[2] - camP[2] };
  float up[3] = { -camUp[0], -camUp[1], -camUp[2] };
  GLfloat rot[16];
  LookAt(point, up, rot);

  int* size = viewport->GetSize();
  glViewport(0,size[1]-100,100,100);

  glClear(GL_DEPTH_BUFFER_BIT);
  // Blender uses cw, vtk uses ccw, can't care enough to edit normals
  glCullFace(GL_FRONT);
  glEnable(GL_CULL_FACE);
  glUseProgram(m_Program);

  glBindVertexArray(m_Vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ibo);

  glUniformMatrix4fv(m_ProgramRot, 1, GL_FALSE, rot);

  glDrawElements(GL_TRIANGLES, NUM_LOWHUMAN_OBJECT_INDEX, GL_UNSIGNED_SHORT, 0);

  glDisable(GL_CULL_FACE);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  return 1;
}

int HumanViewActor::RenderOpaqueGeometry(vtkViewport* viewport)
{
  return 0;
}

int HumanViewActor::RenderTranslucentPolygonalGeometry(vtkViewport* viewport)
{
  return 0;
}

int HumanViewActor::HasTranslucentPolygonalGeometry()
{
  return 0;
}

