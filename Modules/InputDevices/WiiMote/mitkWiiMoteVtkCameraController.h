#ifndef MITK_WIIMOTEVTKCAMERACONTROLLER_H
#define MITK_WIIMOTEVTKCAMERACONTROLLER_H

#include "mitkCameraController.h"
#include "mitkWiiMoteExports.h"

#include "mitkAction.h"
#include "mitkEvent.h"

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

  }; // end class 
} // end namespace mitk
#endif // MITK_WIIMOTEVTKCAMERACONTROLLER_H