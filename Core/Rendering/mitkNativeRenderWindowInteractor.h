#ifndef MITKNATIVERENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722
#define MITKNATIVERENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722

#include "mitkCommon.h"
#include "mitkRenderWindow.h"

class vtkRenderWindowInteractor;

namespace mitk
{

class NativeRenderWindowInteractor : public itk::Object
{
public:
  mitkClassMacro(NativeRenderWindowInteractor, itk::Object);
  
  itkNewMacro(Self);

  virtual void Start();

  void SetMitkRenderWindow(RenderWindow* renderwindow);
  itkGetMacro(MitkRenderWindow, RenderWindow*);

protected:
  NativeRenderWindowInteractor();
  virtual ~NativeRenderWindowInteractor();

  RenderWindow* m_MitkRenderWindow;

  vtkRenderWindowInteractor* m_NativeVtkRenderWindowInteractor;
};

}
#endif /* MITKNATIVERENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722 */

