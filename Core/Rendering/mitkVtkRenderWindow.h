#ifndef MITKVTKRENDERWINDOW_H_HEADER_INCLUDED_C1C53722
#define MITKVTKRENDERWINDOW_H_HEADER_INCLUDED_C1C53722

#include "BaseRenderer.h"
#include <stdlib.h>
#ifdef WIN32
#include <vtkWin32OpenGLRenderWindow.h>
#else
#include <vtkXOpenGLRenderWindow.h>
#endif


namespace mitk
{

#ifdef WIN32
class VtkRenderWindow : public vtkWin32OpenGLRenderWindow
#else
class VtkRenderWindow : public vtkXOpenGLRenderWindow
#endif
{
public:
  // Description:
  // Construct object so that light follows camera motion.
  static VtkRenderWindow *New();

  vtkTypeMacro(VtkRenderWindow,vtkRenderWindow);

  itkTypeMacro(VtkRenderWindow,vtkRenderWindow);

  itkSetObjectMacro(MitkRenderer, BaseRenderer);
  itkGetObjectMacro(MitkRenderer, BaseRenderer);

  virtual void Render();

  inline void MitkRender() { vtkRenderWindow::Render(); };

protected:
  VtkRenderWindow();
  ~VtkRenderWindow();

  BaseRenderer::Pointer m_MitkRenderer;

private:
  VtkRenderWindow(const VtkRenderWindow&);  // Not implemented.
  void operator=(const VtkRenderWindow&);  // Not implemented.
};

}
#endif /* MITKVTKRENDERWINDOW_H_HEADER_INCLUDED_C1C53722 */

