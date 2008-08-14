#include "mitkTDMouseEventThrower.h"
#include "mitkTDMouseEvent.h"
#include "mitkInteractionConst.h"
#include "mitkVector.h"
#include "mitkGlobalInteraction.h"
#include "mitkStateEvent.h"
#include "mitkSpaceNavigatorDriver.h"


mitk::TDMouseEventThrower * mitk::TDMouseEventThrower::GetInstance()
{
  //instance wird nicht initialisiert und kommt mit 0 zurück!
  static TDMouseEventThrower instance;
  return &instance;
}

mitk::TDMouseEventThrower::TDMouseEventThrower()
{
  //init the driver
  SpaceNavigatorDriver* spaceNavigatorDriver = SpaceNavigatorDriver::GetInstance();
}

mitk::TDMouseEventThrower::~TDMouseEventThrower()
{
}

void mitk::TDMouseEventThrower::DeviceChange (long device, long keys, long programmableKeys)
{
}

void mitk::TDMouseEventThrower::KeyDown (int keyCode)
{
  //send the informations to GlobalInteraction
  mitk::Event* e = new mitk::Event(NULL, mitk::Type_TDMouseKeyDown, mitk::BS_LeftButton, keyCode, mitk::Key_none);
  mitk::StateEvent* se = new mitk::StateEvent(mitk::EIDTDMOUSEKEYDOWN, e);
  mitk::GlobalInteraction::GetInstance()->HandleEvent(se);
}

void mitk::TDMouseEventThrower::KeyUp (int keyCode)
{
}

void mitk::TDMouseEventThrower::SensorInput( mitk::Vector3D translation, mitk::Vector3D rotation, mitk::ScalarType angle)
{
  mitk::TDMouseEvent* e = new mitk::TDMouseEvent(mitk::BS_NoButton, translation, rotation, angle);
  mitk::StateEvent* se = new mitk::StateEvent(mitk::EIDTDMOUSEINPUT, e);
  mitk::GlobalInteraction::GetInstance()->HandleEvent(se);
}
