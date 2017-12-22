/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef CAMERAROTATIONCONTROLLER_H_HEADER_INCLUDED_NXYCBIU
#define CAMERAROTATIONCONTROLLER_H_HEADER_INCLUDED_NXYCBIU

#include "mitkBaseController.h"
#include <MitkCoreExports.h>

class vtkCamera;
class vtkRenderWindow;

namespace mitk
{
  class MITKCORE_EXPORT CameraRotationController : public BaseController
  {
  public:
    mitkClassMacro(CameraRotationController, BaseController);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)
      // mitkNewMacro1Param(Self, const char *);

      void RotateCamera();
    void AcquireCamera();

    void SetRenderWindow(vtkRenderWindow *renWin) { m_RenderWindow = renWin; }
  protected:
    CameraRotationController();
    ~CameraRotationController() override;

  private:
    int m_LastStepperValue;
    vtkCamera *m_Camera;
    vtkRenderWindow *m_RenderWindow;
  };
}

#endif
