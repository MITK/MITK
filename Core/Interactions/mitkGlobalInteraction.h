#ifndef GLOBALINTERACTION_H_HEADER_INCLUDED_C152938A
#define GLOBALINTERACTION_H_HEADER_INCLUDED_C152938A

#include "mitkFocusManager.h"
#include "mitkCommon.h"
#include "mitkStateMachine.h"
#include "mitkInteractor.h"
#include <string>
#include <vector>


namespace mitk {
//##ModelId=3E5B33000230
//##Documentation
//## @brief handles all global Events
//## @ingroup Interaction
//##
//## superior statemachine, that spreads the events to all other Interactors
//##
//## Concept of sending events:
//## In this concept of interaction, the statemachines can be divided into two main statemachines:
//## Listeners and Interactors.
//## Listeners only recieve the event to process it, but don't change any data. They want to listen to all events.
//## Interactors do change data according to the recieved event. They do not need to revieve all events, only
//## those they are interested in.
//##
//## To divide these two types of statemachine this class holds three lists:
//## m_ListenerList, m_InteractorList and m_SelectedList
//## The list m_ListenerList holds all listeners.
//## m_InteractorList holds all Interactors, and the List m_SelectedList holds all machines, that were set to SELECTED or SUBSELECTED.
//## First the listeners are informed with the event.
//## Then the selected or subselected Interactors are asked if they can handle that event.
//## They can handle it, if the machine or a submachine has an according transition.
//## They can't handle it, if the event is not defined in that state (has no transition waiting for that event).
//## In that case, all statemachines in m_InteractorList are asked to handle that event.
//## If a machine can handle the event, then it turns into the modus SELECTED or if a submachine could
//## handle the event, then in modus SUBSELECTED and is asked directly when the next event appears.
//##
//##
class GlobalInteraction : public StateMachine
{
  public:
  //##ModelId=3F0177080324
  typedef std::vector<StateMachine*> StateMachineList;
  //##ModelId=3F0177080334
  typedef std::vector<StateMachine*>::iterator StateMachineListIter;

  typedef std::vector<Interactor*> InteractorList;

  typedef std::vector<Interactor*>::iterator InteractorListIter;

  //##ModelId=3EAD420E0088
  GlobalInteraction(const char * type);



  //##Documentation
  //## @brief add an Interactor to the list of all Interactors that are asked for handling an event
  //##
  //## returns true in case of success
  void AddInteractor(Interactor* interactor);

  //##Documentation
  //## @brief remove a certain Interactor from the set of Interactors that are asked for handling an event
  //##
  //## returns true in case of success
  bool RemoveInteractor(Interactor* interactor);

  //##Documentation
  //## @brief add a Listener to the list of all Listeners that are informed of an event
  //##
  //## returns true in case of success
  void AddListener(StateMachine* listener);

  //##Documentation
  //## @brief remove a certain Listener from the set of Listeners that are informed of an event
  //##
  //## returns true in case of success
  bool RemoveListener(StateMachine* listener);

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
  virtual bool ExecuteAction(int actionId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId);

  private:

  //##Documentation
  //##@brief informing all statemachines that are held in the list m_ListenerList
  void InformListeners(mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId);

  //##Documentation
  //##@brief asking the selected Interactor if an event can be handled
  //##
  //## returns false if no Interactor could handle the event
  bool AskSelected(mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId);

  //##Documentation
  //##@brief asking all Interactors, hwo can handle the event
  void AskAllInteractors(mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId);


  //##Documentation
  //## @brief list of all listening statemachines, that want to recieve all events
  StateMachineList m_ListenerList;

  //##Documentation
  //## @brief list of all interactors (statemachine, that change data)
  InteractorList m_InteractorList;

  //##Documentation
  //## @brief list of all interactors, that are in Mode SELECTED or SUBSELECTED
  InteractorList m_SelectedList; //or id of position in vector

  //##ModelId=3EF099E80373
  //##Documentation
  //## @brief holds a list of BaseRenderer and one focused
  FocusManager* m_FocusManager;

};
} // namespace mitk

#endif /* GLOBALINTERACTION_H_HEADER_INCLUDED_C152938A */

