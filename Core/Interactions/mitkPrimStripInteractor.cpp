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


bool mitk::PrimStripInteractor::ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
  bool ok = false;//for return type bool
  
  switch (sideEffectId)
	{
  case SeDONOTHING:
    ok = true;
	  break;
  default:
    (StatusBar::GetInstance())->DisplayText("Message from mitkPrimStripInteractor: I do not understand the SideEffect!", 10000);
    return false;
    //a false here causes the statemachine to undo its last statechange.
    //otherwise it will end up in a different state, but without done SideEffect.
    //if a transition really has no SideEffect, than call donothing
  }
  return ok;
}
