#include "GlobalInteraction.h"
#include "mitkInteractionConst.h"
#include "Event.h"
#include "Focus.h"


//##ModelId=3EAD420E0088
mitk::GlobalInteraction::GlobalInteraction(std::string type)
: StateMachine(type)
{
	//Quickimplementation ... Ivo: No Need of focus, cause glWidget sends the Event now.
	//m_Focus = new mitk::Focus("focus");
}

inline mitk::StateEvent* GenerateEmptyStateEvent(int eventId)
{
    mitk::Event *noEvent = new mitk::Event(NULL,
        mitk::Type_User,
        mitk::BS_NoButton,
		mitk::BS_NoButton,
        mitk::Key_none);
    mitk::StateEvent *stateEvent = new mitk::StateEvent(eventId, noEvent);
    //mitk::StateEvent *stateEvent = new mitk::StateEvent();
	//stateEvent->Set( eventId, noEvent );
	return stateEvent;
}

//##ModelId=3E7F497F01AE
bool mitk::GlobalInteraction::ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
	bool ok = false;
	//if the Event is a PositionEvent, then get the worldCoordinates through Focus
	/*Quickimplementation... no need of focus, since the qglwidget send the event
	if ( (stateEvent->GetEvent() )->GetType()== MouseButtonPress)
	{
		ok = m_Focus->HandleEvent(stateEvent,...);//give it to Focus which calculates accordingly to it's Statemachine the worldcoordinates

		//for debugging
		if (!ok)
			std::cout<<"Error! Sender: PositionEvent;   Message: HandleEvent returned false"<<std::endl;
	}
	*/


	//actually it is very unclean to write numbers in code (EventId's), but here:
	//The xml-File has to be written, with all the EventNames and EventID's,
	//a StateMachine with sideeffects have to be written, so the developer has
	//to make sure, that everything is conclusive... to be changed in constants!
	ok = false;
	switch (sideEffectId)
	{
	case SeDONOTHING:
		//std::cout << "DoNothing"<< std::endl;
		ok = true;
		break;
	case SeINITNEWOBJECT:
		std::cout<<"InitObject"<<std::endl;
        //UNDO
		//Operation:
		//mitk::TriangleROI* triangleROI = new mitk::TriangleROI("TriangleROI");
		{
			//StateMachine change into Object
			//when ready, then generate ID 202
			//202 for finished Object
			mitk::StateEvent* nextStateEvent = GenerateEmptyStateEvent(202);
			ok = HandleEvent(nextStateEvent, objectEventId, groupEventId);
		}
		break;
	case SeINITEDITOBJECT:
		std::cout<<"InitEditObject"<<std::endl;
		{
			//202 for finished editing object
			//inbetween the edit has to be done to the Object. Next StateMachine!!!
			//this event generation can be done in the next StateMachine!
			mitk::StateEvent* nextStateEvent = GenerateEmptyStateEvent(202);
			ok = HandleEvent(nextStateEvent, objectEventId, groupEventId);
		}
		break;
	case SeINITEDITGROUP:
		std::cout<<"InitEditGroup"<<std::endl;
		{
			//202 for finished editing group
			//inbetween the edit has to be done to the Group. Next StateMachine!!!
			//this event generation can be done in the next StateMachine!
			mitk::StateEvent* nextStateEvent = GenerateEmptyStateEvent(202);
			ok = HandleEvent(nextStateEvent, objectEventId, groupEventId);
		}
		break;
	case SeADDPOINT:
		std::cout<<"AddPoint"<<std::endl;
		ok = true;
		break;
	case SeSNAPPING_ADDING:
		std::cout<<"SnappingAdding"<<std::endl;
		ok = true;
		break;
	case SeCHECKELEMENT:
		std::cout<<"CheckPoint"<<std::endl;
		ok = true;
		break;
	case SeCHECKEQUALS1:
		std::cout<<"CheckIfLast"<<std::endl;
		{
			//208 for neutral
			//209 for group selected
			mitk::StateEvent* nextStateEvent = GenerateEmptyStateEvent(208);
			ok = HandleEvent(nextStateEvent, objectEventId, groupEventId);
		}
		break;
	case SeFINISHOBJECT:
		std::cout<<"FinishObject"<<std::endl;
		{
			mitk::StateEvent* nextStateEvent = GenerateEmptyStateEvent(202);
			ok = HandleEvent(nextStateEvent, objectEventId, IncCurrGroupEventId());
		}
		break;
	case SeFINISHGROUP:
		std::cout<<"FinishGroup"<<std::endl;
		//don't forget to increase GroiupEventId
		ok = true;
		break;
	case SeSEARCHOBJECT:
		std::cout<<"SearchObject- Object Found!"<<std::endl;
		//create new event and send to StateMachine, which handles the statechange and the SideEffect
		{
			//203 for nothing picked
			//204 for object picked
			//205 for same object picked
			mitk::StateEvent* nextStateEvent = GenerateEmptyStateEvent(204);//object picked
			ok = HandleEvent(nextStateEvent, objectEventId, groupEventId);
		}
		break;
	case SeSEARCHGROUP:
		std::cout<<"SearchGroup"<<std::endl;
		{
			//203 for neutral; no group found
			//204 for edit group; group found
			mitk::StateEvent* nextStateEvent = GenerateEmptyStateEvent(203);
			ok = HandleEvent(nextStateEvent, objectEventId, groupEventId);
		}
		break;
	case SeSEARCHANOTHEROBJECT:
		std::cout<<"SearchAnotherObject"<<std::endl;
		{
			//200 for New Object Picked
			//205 for same-Check
			//206 for no new and only one selected
			//207 for No New but more than 1 selected
			mitk::StateEvent* nextStateEvent = GenerateEmptyStateEvent(206);
			ok = HandleEvent(nextStateEvent, objectEventId, groupEventId);
		}
		break;
	case SeSELECTPICKEDOBJECT:
		std::cout<<"SelectPickedObject"<<std::endl;
		ok = true;
		break;
	case SeSELECTANOTHEROBJECT:
		std::cout<<"SelectAnotherObject"<<std::endl;
		ok = true;
		break;
	case SeSELECTGROUP:
		std::cout<<"SelectGroup"<<std::endl;
		ok = true;
		break;
	case SeSELECTALL:
		std::cout<<"SelectAll"<<std::endl;
		ok = true;
		break;
	case SeADDSELECTEDTOGROUP:
		std::cout<<"AddSelectedGroup"<<std::endl;
		ok = true;
		break;
	case SeDESELECTOBJECT:
		std::cout<<"DeselectObject"<<std::endl;
		ok = true;
		break;
	case SeDESELECTALL:
		std::cout<<"DeselectAll"<<std::endl;
		ok = true;
		break;
	default:
		//unknown event!
		ok = true;
	}

/*
@todo:
List of all StateMachines/Interactables that are beneath 
the focused BaseRenderer (Focus-StateMachine).
register at DataTree, that if anything changes, the list can be refreshed.
If a new object is generated here, the new object flows into the m_SelectedElements
and the Focus changes.
*/

    //Quickimplementation; is to be changed with List from DataTree
	//send the event to all in List
    for (StateMachineListIter it = m_LocalStateMachines.begin(); it != m_LocalStateMachines.end(); it++)
	{
        if((*it)!=NULL)
            ok = (*it)->HandleEvent(stateEvent, objectEventId, groupEventId);
	}

	return true;//return ok;
}



//##ModelId=3EF099E90065
void mitk::GlobalInteraction::AddStateMachine(mitk::StateMachine* stateMachine)
{
  if ( std::find(m_LocalStateMachines.begin(), m_LocalStateMachines.end(),stateMachine) == m_LocalStateMachines.end() )
  {
	  m_LocalStateMachines.push_back(stateMachine);
  }
}

//##ModelId=3EF099E900D2
bool mitk::GlobalInteraction::RemoveStateMachine(mitk::StateMachine* stateMachine)
{
	// Try find  
	StateMachineListIter position = std::find(m_LocalStateMachines.begin(), m_LocalStateMachines.end(),stateMachine);
	if (position == m_LocalStateMachines.end())
		return false;
	position = m_LocalStateMachines.erase(position);
	return true;
}