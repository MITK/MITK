#include "GlobalInteraction.h"
#include "mitkInteractionConst.h"

//##ModelId=3EAD420E0088
mitk::GlobalInteraction::GlobalInteraction(std::string type)
: StateMachine(type)
{}

//##ModelId=3E7F497F01AE
bool mitk::GlobalInteraction::ExecuteSideEffect(int sideEffectId)
{
	switch (sideEffectId)
	{
	case mitk::DONOTHING:
		return true;
		//break;
	case mitk::INITNEWOBJECT:
        //UNDO
		//Operation:
		//mitk::TriangleROI* triangleROI = new mitk::TriangleROI("TriangleROI");
		return true;
		break;
	default:
		//unknown event!
		return true;;
	}
	return false;
}

