#include "GlobalInteraction.h"
#include "mitkInteractionConst.h"
#include "Event.h"


//##ModelId=3EAD420E0088
mitk::GlobalInteraction::GlobalInteraction(std::string type)
: StateMachine(type)
{}

inline mitk::StateEvent* GenerateEmptyEvent(int eventId)
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
bool mitk::GlobalInteraction::ExecuteSideEffect(int sideEffectId, mitk::Event const* event)
{
	//if the Event is a PositionEvent, then get the worldCoordinates:
	//get the focused BaseRenderer and compute the coordinates
	if (event->GetType()== MouseButtonPress)
	{
		//get BaseRenderer from Focus and ask about Worldcoordinates
	}

	//actually it is very unclean to write numbers in code (EventId's), but here:
	//The xml-File has to be written, with all the EventNames and EventID's, 
	//a StateMachine with sideeffects have to be written, so the developer has 
	//to make sure, that everything is conclusive... to be changed in constants!
	bool ok = false;
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
			mitk::StateEvent* stateEvent = GenerateEmptyEvent(202);
			ok = HandleEvent(stateEvent);
		}
		break;
	case INITEDITOBJECT:
		std::cout<<"InitEditObject"<<std::endl;
		{
			//202 for finished editing object
			//inbetween the edit has to be done to the Object. Next StateMachine!!!
			//this event generation can be done in the next StateMachine!
			mitk::StateEvent* stateEvent = GenerateEmptyEvent(202);
			ok = HandleEvent(stateEvent);
		}
		break;
	case INITEDITGROUP:
		std::cout<<"InitEditGroup"<<std::endl;
		{
			//202 for finished editing group
			//inbetween the edit has to be done to the Group. Next StateMachine!!!
			//this event generation can be done in the next StateMachine!
			mitk::StateEvent* stateEvent = GenerateEmptyEvent(202);
			ok = HandleEvent(stateEvent);
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
			mitk::StateEvent* stateEvent = GenerateEmptyEvent(208);
			ok = HandleEvent(stateEvent);
		}
		break;
	case FINISHOBJECT:
		std::cout<<"FinishObject"<<std::endl;
		{
			mitk::StateEvent* stateEvent = GenerateEmptyEvent(202);
			ok = HandleEvent(stateEvent);
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
			mitk::StateEvent* stateEvent = GenerateEmptyEvent(204);//object picked
			ok = HandleEvent(stateEvent);
		}
		break;
	case SEARCHGROUP:
		std::cout<<"SearchGroup"<<std::endl;
		{
			//203 for neutral; no group found
			//204 for edit group; group found
			mitk::StateEvent* stateEvent = GenerateEmptyEvent(203);
			ok = HandleEvent(stateEvent);
		}
		break;
	case SEARCHANOTHEROBJECT:
		std::cout<<"SearchAnotherObject"<<std::endl;
		{
			//200 for New Object Picked
			//205 for same-Check
			//206 for no new and only one selected
			//207 for No New but more than 1 selected
			mitk::StateEvent* stateEvent = GenerateEmptyEvent(206);
			ok = HandleEvent(stateEvent);
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
