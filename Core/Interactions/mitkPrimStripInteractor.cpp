#include "mitkPrimStripInteractor.h"
#include "mitkStatusBar.h"
#include <mitkInteractionConst.h>
#include "mitkAction.h"

mitk::PrimStripInteractor::PrimStripInteractor(const char * type, DataTreeNode* dataTreeNode)
: HierarchicalInteractor(type, dataTreeNode)
{
}

mitk::PrimStripInteractor::~PrimStripInteractor()
{
}

bool mitk::PrimStripInteractor::ExecuteAction(Action* action, mitk::StateEvent const* stateEvent)
{
  bool ok = false;//for return type bool
  
  switch (action->GetActionId())
	{
  default:
    return Superclass::ExecuteAction(action, stateEvent);
  }
  return ok;
}
