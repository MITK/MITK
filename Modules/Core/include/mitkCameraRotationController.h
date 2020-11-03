/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self)
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
