#include "mitkVtkRenderWindow.h"
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
mitk::VtkRenderWindow* mitk::VtkRenderWindow::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("mitk::VtkRenderWindow");
  if(ret)
  {
    return (mitk::VtkRenderWindow*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new mitk::VtkRenderWindow;
}

//// Construct object so that light follows camera motion.
mitk::VtkRenderWindow::VtkRenderWindow() : m_MitkRenderer(NULL)
{

}

mitk::VtkRenderWindow::~VtkRenderWindow() 
{

}

void mitk::VtkRenderWindow::Render()
{
  if(m_MitkRenderer)
    m_MitkRenderer->Render();
  else
    itkExceptionMacro("MitkRenderer not set.");
}
