#include "GlobalInteraction.h"
#include "mitkInteractionConst.h"
#include "Event.h"
#include "Focus.h"


//##ModelId=3EAD420E0088
mitk::GlobalInteraction::GlobalInteraction(std::string type)
: StateMachine(type)
{
	m_Focus = new mitk::Focus("focus");
}

inline mitk::StateEvent* GenerateEmptyStateEvent(int eventId)
{
	mitk::Event *noEvent = new mitk::Event(User,
									NoButton,
									NoButton,
									Key_none);
	mitk::StateEvent *stateEvent = new mitk::StateEvent();
	stateEvent->Set( eventId, noEvent );
	return stateEvent;
}

//##ModelId=3E7F497F01AE
bool mitk::GlobalInteraction::ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent)
{
	bool ok = false;
	//if the Event is a PositionEvent, then get the worldCoordinates through Focus
	if ( (stateEvent->GetEvent() )->GetType()== MouseButtonPress)
	{
		ok = m_Focus->HandleEvent(stateEvent);//give it to Focus which calculates accordingly to it's Statemachine the worldcoordinates

		//for debugging
		if (!ok)
			std::cout<<"Error! Sender: PositionEvent;   Message: HandleEvent returned false"<<std::endl;
	}

	//actually it is very unclean to write numbers in code (EventId's), but here:
	//The xml-File has to be written, with all the EventNames and EventID's,
	//a StateMachine with sideeffects have to be written, so the developer has
	//to make sure, that everything is conclusive... to be changed in constants!
	ok = false;
	switch (sideEffectId)
	{
	case DONOTHING:
		std::cout << "DoNothing"<< std::endl;
		ok = true;
		break;
	case INITNEWOBJECT:
		std::cout<<"InitObject"<<std::endl;
        //UNDO
		//Operation:
		//mitk::TriangleROI* triangleROI = new mitk::TriangleROI("TriangleROI");
		{
			//StateMachine change into Object
			//when ready, then generate ID 202
			//202 for finished Object
			mitk::StateEvent* nextStateEvent = GenerateEmptyStateEvent(202);
			ok = HandleEvent(nextStateEvent);
		}
		break;
	case INITEDITOBJECT:
		std::cout<<"InitEditObject"<<std::endl;
		{
			//202 for finished editing object
			//inbetween the edit has to be done to the Object. Next StateMachine!!!
			//this event generation can be done in the next StateMachine!
			mitk::StateEvent* nextStateEvent = GenerateEmptyStateEvent(202);
			ok = HandleEvent(nextStateEvent);
		}
		break;
	case INITEDITGROUP:
		std::cout<<"InitEditGroup"<<std::endl;
		{
			//202 for finished editing group
			//inbetween the edit has to be done to the Group. Next StateMachine!!!
			//this event generation can be done in the next StateMachine!
			mitk::StateEvent* nextStateEvent = GenerateEmptyStateEvent(202);
			ok = HandleEvent(nextStateEvent);
		}
		break;
	case ADDPOINT:
		std::cout<<"AddPoint"<<std::endl;
		ok = true;
		break;
	case SNAPPING_ADDING:
		std::cout<<"SnappingAdding"<<std::endl;
		ok = true;
		break;
	case CHECKPOINT:
		std::cout<<"CheckPoint"<<std::endl;
		ok = true;
		break;
	case CHECKIFLAST:
		std::cout<<"CheckIfLast"<<std::endl;
		{
			//208 for neutral
			//209 for group selected
			mitk::StateEvent* nextStateEvent = GenerateEmptyStateEvent(208);
			ok = HandleEvent(nextStateEvent);
		}
		break;
	case FINISHOBJECT:
		std::cout<<"FinishObject"<<std::endl;
		{
			mitk::StateEvent* nextStateEvent = GenerateEmptyStateEvent(202);
			ok = HandleEvent(nextStateEvent);
		}
		break;
	case FINISHGROUP:
		std::cout<<"FinishGroup"<<std::endl;
		ok = true;
		break;
	case SEARCHOBJECT:
		std::cout<<"SearchObject- Object Found!"<<std::endl;
		//create new event and send to StateMachine, which handles the statechange and the SideEffect
		{
			//203 for nothing picked
			//204 for object picked
			//205 for same object picked
			mitk::StateEvent* nextStateEvent = GenerateEmptyStateEvent(204);//object picked
			ok = HandleEvent(nextStateEvent);
		}
		break;
	case SEARCHGROUP:
		std::cout<<"SearchGroup"<<std::endl;
		{
			//203 for neutral; no group found
			//204 for edit group; group found
			mitk::StateEvent* nextStateEvent = GenerateEmptyStateEvent(203);
			ok = HandleEvent(nextStateEvent);
		}
		break;
	case SEARCHANOTHEROBJECT:
		std::cout<<"SearchAnotherObject"<<std::endl;
		{
			//200 for New Object Picked
			//205 for same-Check
			//206 for no new and only one selected
			//207 for No New but more than 1 selected
			mitk::StateEvent* nextStateEvent = GenerateEmptyStateEvent(206);
			ok = HandleEvent(nextStateEvent);
		}
		break;
	case SELECTPICKEDOBJECT:
		std::cout<<"SelectPickedObject"<<std::endl;
		ok = true;
		break;
	case SELECTANOTHEROBJECT:
		std::cout<<"SelectAnotherObject"<<std::endl;
		ok = true;
		break;
	case SELECTGROUP:
		std::cout<<"SelectGroup"<<std::endl;
		ok = true;
		break;
	case SELECTALL:
		std::cout<<"SelectAll"<<std::endl;
		ok = true;
		break;
	case ADDSELECTEDTOGROUP:
		std::cout<<"AddSelectedGroup"<<std::endl;
		ok = true;
		break;
	case DESELECTOBJECT:
		std::cout<<"DeselectObject"<<std::endl;
		ok = true;
		break;
	case DESELECTALL:
		std::cout<<"DeselectAll"<<std::endl;
		ok = true;
		break;
	default:
		//unknown event!
		ok = true;
	}
	return ok;
}

void mitk::GlobalInteraction::ExecuteOperation(mitk::Operation* operation)
{
	NULL;
}
