#ifndef MITKVTKRENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722
#define MITKVTKRENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722

//## #include <stdlib.h>
#include "vtkRenderWindowInteractor.h"
#include "mitkBaseRenderer.h"

//##ModelId=3E6D600E0193
//##Documentation
//## @brief mitk::VtkRenderWindowInteractor is an agent class between mitk and a GUI 
//## dependant RenderWindowInteractor like VtkQRenderWindowInteractor for QT
//## (Bridge-pattern)
//## @ingroup NavigationControl
namespace mitk {

class VtkRenderWindowInteractor : public ::vtkRenderWindowInteractor
{
  mitk::BaseRenderer::Pointer m_MitkRenderer;
public:
  itkTypeMacro(VtkRenderWindowInteractor,::vtkRenderWindowInteractor);

  itkSetObjectMacro(MitkRenderer, mitk::BaseRenderer);
  itkGetObjectMacro(MitkRenderer, mitk::BaseRenderer);
  virtual void Resize(int w, int h) = 0;
};

} // mitk

#endif /* MITKVTKRENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722 */

