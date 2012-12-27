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

#include <MitkExports.h>
#include "mitkBaseController.h"

class vtkCamera;
class vtkRenderWindow;

namespace mitk {


class MITK_CORE_EXPORT CameraRotationController : public BaseController
{
public:
  mitkClassMacro(CameraRotationController,BaseController);
  itkNewMacro(Self);
  mitkNewMacro1Param(Self, const char *);

  void RotateCamera();
  void AcquireCamera();

  void SetRenderWindow(vtkRenderWindow * renWin)
  {
    m_RenderWindow = renWin;
  }

  virtual bool ExecuteAction( Action* action, mitk::StateEvent const* stateEvent );

protected:
  CameraRotationController(const char * type = NULL);
  virtual ~CameraRotationController();

private:
  int m_LastStepperValue;
  vtkCamera* m_Camera;
  vtkRenderWindow* m_RenderWindow;
};

}

#endif
