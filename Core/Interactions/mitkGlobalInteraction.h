#ifndef GLOBALINTERACTION_H_HEADER_INCLUDED_C152938A
#define GLOBALINTERACTION_H_HEADER_INCLUDED_C152938A

#include "mitkFocusManager.h"
#include "mitkCommon.h"
#include "mitkStateMachine.h"
#include "mitkRoi.h"
#include <string>
#include <vector>


namespace mitk {
//##ModelId=3E5B33000230
//##Documentation
//## @brief handles all global Events ("not specified on one object"-Events).
//## @ingroup Interaction
//## 
//## superior statemachine, that handles the events and asks all other ROI's
//##
//## GroupEventId and ObjectEventId are there 
//## for to have the opportunity to recall an operation
//## operation by operation (different ObjectID, same GroupId)
//## or interactionstep by interactionstep (different GroupId)
//## for to know which operation has to have which Object or Group EventID
//## the ID's have to be given by the global StateMachine to the local StateMachines
class GlobalInteraction : public StateMachine
{
	public:
	//##ModelId=3F0177080324
	typedef std::vector<StateMachine*> StateMachineList;
	//##ModelId=3F0177080334
	typedef std::vector<StateMachine*>::iterator StateMachineListIter;

	//##ModelId=3EAD420E0088
	GlobalInteraction(std::string type);

    
  //##ModelId=3EF099E90065
	//##Documentation
	//## @brief add a Statemachine to the set of StateMachines that are asked for handling an event
	//##
	//## returns true in case of success
	void AddStateMachine(StateMachine* stateMachine);

  //##ModelId=3EF099E900D2
	//##Documentation
	//## @brief remove a certain Statemachine from the set of StateMachines that are asked for handling an event
	//##
	//## returns true in case of success
	bool RemoveStateMachine(StateMachine* stateMachine);

  //##Documentation
	//## @brief adds an element in the list in FocusManager
  //##
  //## true if success, false if the element is already in list
  bool AddFocusElement(FocusManager::FocusElement* element);

  //##Documentation
	//## @brief Removes an element in FocusManager
  //##
  //## true if success, false if the element was not in the list
  bool RemoveFocusElement(FocusManager::FocusElement* element);

  //##Documentation
	//## @brief Returns the focused Element in FocusManager
  const FocusManager::FocusElement* GetFocus();

  //##Documentation
	//## @brief Sets the given Element to focused
  //##
  //## returns true if the given element was found and focused
  bool SetFocus(FocusManager::FocusElement* element);

  //##Documentation
	//## @brief Returns the pointer to the FocusManager
  //##
  //## to add the observer for an event
  mitk::FocusManager* mitk::GlobalInteraction::GetFocusManager();

	protected:

	//##ModelId=3E7F497F01AE
    virtual bool ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId);

	private:

    //##ModelId=3F0185C80306
	//##Documentation
	//## After m_Roi the next quickimplementation! Thought is a list of StateMachines to be asked.
	//## take the information from BaseRenderer and parse through the tree and save all interactables
	//## if Tree changes, then build up a new list.
	//## if a new object is added to the tree, then pudh the old one in Undo and set the new 
	//## to the current (m_selectedElements)...
	StateMachineList m_LocalStateMachines;

  //##ModelId=3EF099E80373
  //##Documentation
  //## @brief holds a list of BaseRenderer and one focused
  FocusManager* m_FocusManager;

  //##ModelId=3EDDD763029D
  //Group* m_Group;


};
} // namespace mitk

#endif /* GLOBALINTERACTION_H_HEADER_INCLUDED_C152938A */



