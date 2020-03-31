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

#include <MitkCoreExports.h>
#include "mitkBaseController.h"

class vtkCamera;
class vtkRenderWindow;

namespace mitk {


class MITKCORE_EXPORT CameraRotationController : public BaseController
{
public:
  mitkClassMacro(CameraRotationController,BaseController);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  //mitkNewMacro1Param(Self, const char *);

  void RotateCamera();
  void Elevate();

  void RotateToAngle(double angle);
  void AcquireCamera();

  void SetRenderWindow(vtkRenderWindow * renWin)
  {
    m_RenderWindow = renWin;
  }

  Stepper::Pointer GetElevationSlice() {
    return m_ElevationSlice;
  }

  //virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent) override;
  void RotateCameraToTransformationAngles();
  void RotateCameraBack();
  void Mirror(bool horizontal);
  void ResetTransformationAngles();

  double getRoll() { return m_Roll; }
  double getAzimuth() { return m_Azimuth; }

protected:
  CameraRotationController();
  virtual ~CameraRotationController();

  int m_LastStepperValue;
  int m_ElevateLastStepperValue;

  vtkCamera* m_Camera;
  vtkRenderWindow* m_RenderWindow;

  Stepper::Pointer m_ElevationSlice;

  double m_Roll;
  double m_Azimuth;
};

}

#endif
