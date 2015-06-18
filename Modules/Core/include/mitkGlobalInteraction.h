/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef GLOBALINTERACTION_H_HEADER_INCLUDED_C152938A
#define GLOBALINTERACTION_H_HEADER_INCLUDED_C152938A

#include "mitkFocusManager.h"
#include <MitkCoreExports.h>
#include "mitkStateMachineFactory.h"
#include "mitkEventMapper.h"
#include "mitkInteractor.h"

namespace mitk {

  class PositionEvent;

  //##Documentation
  //## @brief handles all global Events
  //##
  //## superior statemachine, that spreads the events to all other interactors
  //##
  //## Initialization
  //## Attention: GlobalInteraction <strong>must</strong> be initialized by the Initialize() method
  //## before usage by giving it an XML scheme. Possibilities are giving it an empty string (default),
  //## the filename of an XML file or the actual XML content as std::string. If an empty string is given,
  //## the content is tried to be loaded from the default file location.
  //##
  //## Concept of sending events:
  //## In this concept of interaction, the statemachines can be divided into two main statemachines:
  //## Listeners and interactors.
  //## Listeners only receive the event to process it, but don't change any data. They want to listen to all events.
  //## Interactors do change data according to the received event. They do not need to receive all events, only
  //## those they are interested in.
  //## Additional to that a EVENT_NOTIFICATION_POLICY can be set. This can be either INFORM_MULTIPLE (the event is passed
  //## to all listeners) or INFORM_ONE (event is passed to the listener which can handle the event best and only to this one).
  //##
  //## To divide these two types of statemachine this class holds three lists and two maps:
  //## m_ListenerList, m_InteractorList, m_SelectedList and m_InteractorRelevanceMap and m_ListenerRelevanceMap
  //## The list m_ListenerList holds all listeners.
  //## m_InteractorList holds all interactors, and the List m_SelectedList holds all machines, that were set to SELECTED or SUBSELECTED.
  //## m_InteractorRelevanceMap and m_ListenerRelevanceMap map values returned from CanHandleEvent to the asked Interactors and Listeners.
  //## Through m_InteractorRelevanceMap stepping through interactors, that were not selected and could handle that event, can be done.
  //## Through m_ListenerRelevanceMap the listener which can handle the event best can be determined. In case of the INFORM_ONE notification policy
  //## the event is passed to just this listener
  //##
  //## First the listeners are informed with the event.
  //## Then the selected or subselected interactors are asked if they can handle that event.
  //## They can handle it, if the mode of the interactor after HandleEvent(..) is still in SMSELECTED or SMSUBSELECTED.
  //## They can't handle it, if the mode changed to SMDESELECTED. Then the interactor is removed from the selected-list.
  //## In that case, all interactors are asked to calculate and return their area of Relevance.
  //## An iterator is held on one interactor in the map. With the iterator, the map can be looped through so
  //## so that several geometric objects, that lie on top of each other, can be selected.
  //## @ingroup Interaction

  /**
   * \deprecatedSince{2013_03} GlobalInteraction is deprecated. It is replaced by mitk::Dispatcher.
   *  Please use the new implementation described in \see DataInteractionPage .
   */

 class MITKCORE_EXPORT GlobalInteraction : public StateMachine
  {
  public:
    mitkClassMacro(GlobalInteraction, StateMachine);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    typedef std::vector<StateMachine::Pointer>  StateMachineList;
    typedef std::vector<StateMachine*>  StateMachineCPointerList;
    typedef StateMachineList::iterator          StateMachineListIter;
    typedef StateMachineCPointerList::iterator          StateMachineCPointerListIter;
    typedef std::vector<Interactor::Pointer>    InteractorList;
    typedef InteractorList::iterator            InteractorListIter;
    typedef std::multimap<float, Interactor::Pointer, std::greater<double> > InteractorMap;
    typedef std::multimap<float, StateMachine::Pointer, std::greater<double> > StateMachineMap;
    typedef InteractorMap::iterator             InteractorMapIter;
    typedef StateMachineMap::iterator             StateMachineMapIter;

    /**
      * Enum for setting the event notification policy of the GlobalInteraction.
      */
    enum EVENT_NOTIFICATION_POLICY
    {
      INFORM_MULTIPLE, /** For setting that all registered listeners are informed */
      INFORM_ONE /** For setting that just the listener that can handle the event best is informed */
    };

    //##Documentation
    //## @brief add an Interactor to the list of all interactors that are asked for handling an event
    //##
    //## returns true in case of success
    void AddInteractor(Interactor* interactor);

    //##Documentation
    //## @brief remove a certain Interactor from the set of interactors that are asked for handling an event
    //##
    //## returns true in case of success
    bool RemoveInteractor(Interactor* interactor);

    //##Documentation
    //## @brief returns true, if the given interactor is already added to the Interactor-List
    bool InteractorRegistered (Interactor* interactor);

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
    //## @brief returns true, if the given interactor is already added to the Listener-List
    bool ListenerRegistered (StateMachine* listener);

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
    FocusManager::FocusElement* GetFocus();

    //##Documentation
    //## @brief Sets the given Element to focused
    //##
    //## returns true if the given element was found and focused
    bool SetFocus(FocusManager::FocusElement* element);

    //##Documentation
    //## @brief Returns the pointer to the FocusManager
    //##
    //## to add the observer for an event
    FocusManager* GetFocusManager();

    //##Documentation
    //## @brief Returns the pointer to the EventMapper
    //##
    //## to add an addon
    EventMapper* GetEventMapper();

    /**
    * @brief Return StateMachineFactory
    **/
    StateMachineFactory* GetStateMachineFactory();

    /**
    * @brief Returns the StartState of the StateMachine with the name type;
    *
    * Asks member StateMachineFactory for the StartState.
    * Returns NULL if no entry with name type is found.
    **/
    State* GetStartState(const char* type);

    //##Documentation
    //## @brief Returns the global (singleton) instance of
    //## GlobalInteraction. Create it, if it does not exist.
    static GlobalInteraction* GetInstance();

    //##Documentation
    //## @brief Initializes the global (singleton) instance of
    //## GlobalInteraction via an XML string. Must! be done before usage. Can be done only once.
    //## Can be used with an empty string (default), a file name with path, or the actual XML content as string.
    bool Initialize(const char* globalInteractionName, const std::string XMLBehaviorInput = "");

    //##Documentation
    //## @brief Check if GlobalInteraction has already been initialized. Init must! be done before usage.
    bool IsInitialized() {return m_IsInitialized;}

    /**
      * @brief Set the policy of how the global interaction informs listeners and interactors
      *
      * INFORM_MULTIPLE broadcasts the event to all listeners and interactors that can handle the event
      * INFORM_ONE only informs the listener or interactor which can handle the event best
      **/
    void SetEventNotificationPolicy(EVENT_NOTIFICATION_POLICY);

    /**
      * Return the current set eventspreading policy
      * @returns the current event spreading policy
      **/
    EVENT_NOTIFICATION_POLICY GetEventNotificationPolicy() const;


    //so that the interactors can call AddToSelectedInteractors() and RemoveFromSelectedInteractors()
    friend class Interactor;

  protected:
    /**
    * @brief Default Constructor with type to load the StateMachinePattern of the StateMachine
    * @param XMLbehaviorFile the file which contains the statemachine and event patterns
    * @param type the name of the statemachine pattern this class shall use
    **/
    GlobalInteraction();

    /**
    * @brief Default destructor.
    **/
    ~GlobalInteraction();

    virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent) override;

    /*
    *@brief adds the given interactor to the list of selected interactors.
    * This list is asked first to handle an event.
    */
    virtual bool AddToSelectedInteractors(Interactor* interactor);

    /*
    *@brief removes the given interactor from the list of selected interactors
    * This list is asked first to handle an event.
    */
    virtual bool RemoveFromSelectedInteractors(Interactor* interactor);

  private:

    //##Documentation
    //##@brief informing all statemachines that are held in the list m_ListenerList
    void InformListeners(mitk::StateEvent const* stateEvent);

    //##Documentation
    //##@brief asking the selected Interactor if an event can be handled
    //##
    //## returns false if no Interactor could handle the event
    bool AskSelected(mitk::StateEvent const* stateEvent);

    //##Documentation
    //##@brief asking next interactor of m_RelevanceMap
    bool AskCurrentInteractor(mitk::StateEvent const* stateEvent);

    //##Documentation
    //##@brief filling m_InteractorRelevanceMap
    //##
    //## @ params threshold: if the calculated Relevance value is above threshold, then add it to the map
    void FillInteractorRelevanceMap(mitk::StateEvent const* stateEvent, float threshold);

    //##Documentation
    //##@brief filling m_ListenerRelevanceMap
    //##
    //## @ params threshold: if the calculated Relevance value is above threshold, then add it to the map
    void FillListenerRelevanceMap(mitk::StateEvent const* stateEvent, float threshold);

    void RemoveFlaggedListeners();

    StateMachineCPointerList m_ListenersFlaggedForRemoval;

    //##Documentation
    //## @brief list of all listening statemachines, that want to receive all events
    StateMachineList m_ListenerList;

    //##Documentation
    //## @brief list of all interactors (statemachine, that change data)
    InteractorList m_InteractorList;

    //##Documentation
    //## @brief list of all interactors, that are in Mode SELECTED or SUBSELECTED
    InteractorList m_SelectedList;

    //##Documentation
    //## @brief map for sorting all interactors by the value returned from CanHandleEvent(..).
    //##
    //## With that list certain interactors can be looped through like diving through layers
    InteractorMap m_InteractorRelevanceMap;

    //##Documentation
    //## @brief map for sorting all listeners by the value returned from CanHandleEvent(..).
    //##
    //## With that list certain listeners can be looped through like diving through layers
    StateMachineMap m_ListenerRelevanceMap;

    //##Documentation
    //## @brief iterator on an entry in m_RelevanceMap for stepping through interactors
    InteractorMapIter m_CurrentInteractorIter;

    //##Documentation
    //## @brief holds a list of BaseRenderer and one focused
    FocusManager::Pointer m_FocusManager;

    /**
    * @brief StatemachineFactory loads statemachine patterns and provides start states
    **/
    StateMachineFactory* m_StateMachineFactory;

    /**
    * @brief EventMapper loads event patterns
    **/
    EventMapper* m_EventMapper;

    bool m_CurrentlyInInformListenersLoop;
    bool m_CurrentlyInInformInteractorsLoop;
    bool m_IsInitialized;

    EVENT_NOTIFICATION_POLICY m_EventNotificationPolicy;
  };
} // namespace mitk

#endif /* GLOBALINTERACTION_H_HEADER_INCLUDED_C152938A */


