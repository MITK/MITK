#ifndef HUMAN_VIEW_MAPPER_H_INCLUDED
#define HUMAN_VIEW_MAPPER_H_INCLUDED

#include <MitkQtWidgetsExports.h>

#include <vtk_glew.h> // ? vtkOpenGL.h sends to sysyem opengl, why vtk uses two opengl api?

#include <vtkActor2D.h>

class vtkCamera;

class MITKQTWIDGETS_EXPORT HumanViewActor : public vtkActor2D
{
public:
  static HumanViewActor* New();
  vtkTypeMacro(HumanViewActor, vtkActor2D);

  HumanViewActor();
  virtual ~HumanViewActor();
  void AllocateBuffers();

  int RenderOverlay(vtkViewport* viewport) VTK_OVERRIDE;
  int RenderOpaqueGeometry(vtkViewport* viewport) VTK_OVERRIDE;
  int RenderTranslucentPolygonalGeometry(vtkViewport* viewport) VTK_OVERRIDE;

  int HasTranslucentPolygonalGeometry() VTK_OVERRIDE;

  vtkCamera* mainCamera;

protected:
  GLuint m_Vao;
  GLuint m_Vbo[2];
  GLuint m_Ibo;

  GLuint m_Program;
  GLuint m_ProgramRot;

private:
  bool m_FirstRender = true;
};

#endif

