#ifndef CAMERAROTATIONCONTROLLER_H_HEADER_INCLUDED_NXYCBIU
#define CAMERAROTATIONCONTROLLER_H_HEADER_INCLUDED_NXYCBIU

#include "mitkCommon.h"
#include "mitkBaseController.h"

class vtkCamera;


namespace mitk {

class RenderWindow;

class CameraRotationController : public BaseController
{
public:
  mitkClassMacro(CameraRotationController,BaseController);

  CameraRotationController(const char * type = NULL);
  virtual ~CameraRotationController();
  void RotateCamera();
  void AcquireCamera();
  itkSetMacro(RenderWindow, RenderWindow*);
  itkGetConstMacro(RenderWindow, const RenderWindow*);

  virtual bool ExecuteAction( Action* action, mitk::StateEvent const* stateEvent );

private:
  int m_LastStepperValue;
  vtkCamera* m_Camera;
  const RenderWindow* m_RenderWindow;
};

}

#endif
