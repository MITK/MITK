#include "mitkNativeRenderWindowInteractor.h"
#include "mitkVtkRenderWindow.h"

#include <vtkRenderWindowInteractor.h>

mitk::NativeRenderWindowInteractor::NativeRenderWindowInteractor() : m_MitkRenderWindow(NULL), m_NativeVtkRenderWindowInteractor(NULL)
{
  m_NativeVtkRenderWindowInteractor = vtkRenderWindowInteractor::New();
}

mitk::NativeRenderWindowInteractor::~NativeRenderWindowInteractor() 
{

}

void mitk::NativeRenderWindowInteractor::SetMitkRenderWindow(mitk::RenderWindow* renderwindow)
{
  m_MitkRenderWindow = renderwindow;
  if(m_MitkRenderWindow != NULL)
    m_NativeVtkRenderWindowInteractor->SetRenderWindow(m_MitkRenderWindow->GetVtkRenderWindow());
}

void mitk::NativeRenderWindowInteractor::Start()
{
  if(m_MitkRenderWindow != NULL)
    m_NativeVtkRenderWindowInteractor->Start();
}
