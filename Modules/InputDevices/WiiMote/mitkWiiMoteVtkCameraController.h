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
#ifndef MITK_WIIMOTEVTKCAMERACONTROLLER_H
#define MITK_WIIMOTEVTKCAMERACONTROLLER_H

// export
#include "mitkWiiMoteExports.h"

// mitk
#include "mitkCameraController.h"
#include "mitkAction.h"
#include "mitkEvent.h"
#include "mitkBaseRenderer.h"
#include "mitkWiiMoteIREvent.h"
#include "mitkWiiMoteCalibrationEvent.h"

namespace mitk
{
  class mitkWiiMote_EXPORT WiiMoteVtkCameraController : public CameraController
  {

  public:

    //SmartPointer Macros
    mitkClassMacro(WiiMoteVtkCameraController, CameraController);
    itkNewMacro(Self);

  protected:

    WiiMoteVtkCameraController();
    ~WiiMoteVtkCameraController();

  private:

    // head tracking
    bool OnWiiMoteInput(mitk::Action* a, const mitk::StateEvent* e);
    bool ResetView(mitk::Action* a, const mitk::StateEvent* e);

    bool m_ClippingRangeIsSet;
    double m_CurrentElevationAngle;
    double m_CurrentAzimuthAngle;

    // calibration
    bool m_Calibrated;
    double m_SensitivityXMAX;
    double m_SensitivityXMIN;
    double m_SensitivityYMAX;
    double m_SensitivityYMIN;
    double m_SensitivityX;
    double m_SensitivityY;

    bool InitCalibration(mitk::Action* a, const mitk::StateEvent* e);
    bool Calibration(mitk::Action* a, const mitk::StateEvent* e);
    bool FinishCalibration(mitk::Action* a, const mitk::StateEvent* e);

    // slice scrolling
    mitk::BaseRenderer::Pointer m_AxialBR;
    double m_InitialScrollValue;
    int m_UpdateFrequency;

  }; // end class
} // end namespace mitk
#endif // MITK_WIIMOTEVTKCAMERACONTROLLER_H
