#include "mitkAffineInteractor.h"
#include "mitkInteractionConst.h"
#include <mitkDataTreeNode.h>

mitk::AffineInteractor::AffineInteractor(std::string type, DataTreeNode* dataTreeNode)
	 : Interactor(type, dataTreeNode)
{
}

bool mitk::AffineInteractor::ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
	bool ok = false;//for return type bool

  /*Each case must watch the type of the event!*/
  switch (sideEffectId)
	{
  case SeTRANSLATE:
    ok = true;
	  break;
	case SeROTATE:
    ok = true;
    break;
  default:
    ok = true;
  }

return ok;
}
