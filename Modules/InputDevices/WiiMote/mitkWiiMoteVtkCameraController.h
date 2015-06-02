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
#include <MitkWiiMoteExports.h>

// mitk
#include "mitkCameraController.h"
#include "mitkAction.h"
#include "mitkEvent.h"
#include "mitkBaseRenderer.h"
#include "mitkWiiMoteIREvent.h"
#include "mitkWiiMoteCalibrationEvent.h"

namespace mitk
{

/**
 * \deprecatedSince{2015_05} WiiMoteVtkCameraController is deprecated. It will be removed in the next release.
 */
  class mitkWiiMote_EXPORT WiiMoteVtkCameraController : public CameraController
  {

  public:

    //SmartPointer Macros
    mitkClassMacro(WiiMoteVtkCameraController, CameraController);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

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
