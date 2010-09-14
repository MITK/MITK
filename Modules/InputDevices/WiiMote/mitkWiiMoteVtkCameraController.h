#ifndef MITK_WIIMOTEVTKCAMERACONTROLLER_H
#define MITK_WIIMOTEVTKCAMERACONTROLLER_H

#include "mitkCameraController.h"
#include "mitkInputDevicesExports.h"

#include "mitkAction.h"
#include "mitkEvent.h"

#include "mitkWiiMoteEvent.h"

namespace mitk
{
  class mitkInputDevices_EXPORT WiiMoteVtkCameraController : public CameraController
  {

  public:

    //SmartPointer Macros
    mitkClassMacro(WiiMoteVtkCameraController, CameraController);
    itkNewMacro(Self);

  protected: 

    WiiMoteVtkCameraController();
    ~WiiMoteVtkCameraController();

  private:

    bool OnWiiMoteInput(mitk::Action* a, const mitk::StateEvent* e);

    bool m_ClippingRangeIsSet;

  }; // end class 
} // end namespace mitk
#endif // MITK_WIIMOTEVTKCAMERACONTROLLER_H