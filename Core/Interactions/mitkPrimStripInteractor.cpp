#include "mitkPrimStripInteractor.h"
#include "mitkStatusBar.h"
#include <mitkInteractionConst.h>

mitk::PrimStripInteractor::PrimStripInteractor(const char * type, DataTreeNode* dataTreeNode)
: Interactor(type, dataTreeNode)
{
}

mitk::PrimStripInteractor::~PrimStripInteractor()
{
}

int mitk::PrimStripInteractor::GetId()
{
  return m_Id;
}


bool mitk::PrimStripInteractor::ExecuteAction(int actionId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
  bool ok = false;//for return type bool
  
  switch (actionId)
	{
  case AcDONOTHING:
    ok = true;
	  break;
  default:
    mitk::StatusBar::DisplayText("Message from mitkPrimStripInteractor: I do not understand the Action!", 10000);
    return false;
    //a false here causes the statemachine to undo its last statechange.
    //otherwise it will end up in a different state, but without done Action.
    //if a transition really has no Action, than call donothing
  }
  return ok;
}
