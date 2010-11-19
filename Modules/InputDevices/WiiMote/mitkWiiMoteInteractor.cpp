#include <mitkWiiMoteInteractor.h>

// mitk
#include <mitkInteractionConst.h>
#include <mitkWiiMoteAllDataEvent.h>

mitk::WiiMoteInteractor::WiiMoteInteractor(const char* type, DataNode* dataNode)
: Interactor(type, dataNode)
{
  CONNECT_ACTION(mitk::AcONWIIMOTEINPUT,OnWiiMoteInput);
}

mitk::WiiMoteInteractor::~WiiMoteInteractor()
{

}

bool mitk::WiiMoteInteractor::OnWiiMoteActivateButton(Action* action, const mitk::StateEvent* stateEvent)
{

return true;
}

bool mitk::WiiMoteInteractor::OnWiiMoteInput(Action* action, const mitk::StateEvent* stateEvent)
{
  const mitk::WiiMoteAllDataEvent* wiiMoteEvent = dynamic_cast<const mitk::WiiMoteAllDataEvent*>(stateEvent->GetEvent());
  float pitchSpeed = wiiMoteEvent->GetPitchSpeed();
  return true;
}

bool mitk::WiiMoteInteractor::OnWiiMoteReleaseButton(Action* action, const mitk::StateEvent* stateEvent)
{
return true;
}