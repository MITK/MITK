/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKVTKRENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722
#define MITKVTKRENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722

//## #include <stdlib.h>
#include "vtkRenderWindowInteractor.h"
#include "mitkBaseRenderer.h"

namespace mitk {

//##ModelId=3E6D600E0193
//##Documentation
//## @brief mitk::VtkRenderWindowInteractor is an agent class between mitk and a GUI 
//## dependant RenderWindowInteractor like VtkQRenderWindowInteractor for QT
//## (Bridge-pattern)
//## @ingroup NavigationControl
class VtkRenderWindowInteractor : public ::vtkRenderWindowInteractor
{

protected:
  itk::WeakPointer<BaseRenderer> m_MitkRenderer;

public:
  VtkRenderWindowInteractor() : m_MitkRenderer(NULL){};
  itkTypeMacro(VtkRenderWindowInteractor,::vtkRenderWindowInteractor);

  itkSetObjectMacro(MitkRenderer, mitk::BaseRenderer);
  itkGetObjectMacro(MitkRenderer, mitk::BaseRenderer);
  virtual void Resize(int w, int h) = 0;

  virtual ~VtkRenderWindowInteractor()
  {
  };
};

} // mitk

#endif /* MITKVTKRENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722 */

