#ifndef MITKVTKRENDERWINDOW_H_HEADER_INCLUDED_C1C53722
#define MITKVTKRENDERWINDOW_H_HEADER_INCLUDED_C1C53722

#include "mitkBaseRenderer.h"
#include <stdlib.h>
#ifdef WIN32
  #include <vtkWin32OpenGLRenderWindow.h>
#else
  #include <vtkXOpenGLRenderWindow.h>
#endif

namespace mitk
{

#ifdef WIN32
  #define PARENTCLASS vtkWin32OpenGLRenderWindow
#else
  #define PARENTCLASS vtkXOpenGLRenderWindow
#endif

class VtkRenderWindow : public PARENTCLASS
#undef PARENTCLASS
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
  virtual ~VtkRenderWindow();

  BaseRenderer::Pointer m_MitkRenderer;

private:
  VtkRenderWindow(const VtkRenderWindow&);  // Not implemented.
  void operator=(const VtkRenderWindow&);  // Not implemented.
};

}
#endif /* MITKVTKRENDERWINDOW_H_HEADER_INCLUDED_C1C53722 */

