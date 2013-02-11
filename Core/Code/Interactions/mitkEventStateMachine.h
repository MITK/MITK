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

#ifndef MITKEVENTSTATEMACHINE_H_
#define MITKEVENTSTATEMACHINE_H_

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include "mitkEventHandler.h"

#include <MitkExports.h>
#include <string>

namespace mitk
{
  class StateMachineContainer;
  class StateMachineAction;
  class InteractionEvent;
  class StateMachineState;
  class DataNode;

  /**
   * \class TActionFunctor
   * \brief Base class of ActionFunctors, to provide an easy to connect actions with functions.
   */

  class MITK_CORE_EXPORT TActionFunctor
  {
  public:
    virtual bool DoAction(StateMachineAction*, InteractionEvent*)=0;
    virtual ~TActionFunctor()
    {
    }
  };

  /**
   * \class TSpecificActionFunctor
   * Specific implementation of ActionFunctor class, implements a reference to the function which is to be executed. It takes two arguments:
   * StateMachineAction - the action by which the function call is invoked, InteractionEvent - the event that caused the transition.
   */
  template<class T>
  class TSpecificActionFunctor: public TActionFunctor
  {
  public:

    TSpecificActionFunctor(T* object, bool (T::*memberFunctionPointer)(StateMachineAction*, InteractionEvent*)) :
        m_Object(object), m_MemberFunctionPointer(memberFunctionPointer)
    {
    }

    virtual ~TSpecificActionFunctor()
    {
    }
    virtual bool DoAction(StateMachineAction* action, InteractionEvent* event)
    {
      return (*m_Object.*m_MemberFunctionPointer)(action, event);// executes member function
    }

  private:
    T* m_Object;
    bool (T::*m_MemberFunctionPointer)(StateMachineAction*, InteractionEvent*);
  };


  /** Macro that can be used to connect a StateMachineAction with a function.
   *  It assumes that there is a typedef Classname Self in classes that use this macro, as is provided by e.g. mitkClassMacro
   */
#define CONNECT_FUNCTION(a, f) \
    EventStateMachine::AddActionFunction(a, new TSpecificActionFunctor<Self>(this, &Self::f));

  /**
   * \class EventStateMachine
   *
   * \brief Super-class that provides the functionality of a StateMachine to DataInteractors.
   *
   * A state machine is created by loading a state machine pattern. It consists of states, transitions and action.
   * The state represent the current status of the interaction, transitions are means to switch between states. Each transition
   * is triggered by an event and it is associated with actions that are to be executed when the state change is performed.
   *
   */

  class MITK_CORE_EXPORT EventStateMachine: public virtual EventHandler
  {

  public:
    mitkClassMacro(EventStateMachine, EventHandler);
    itkNewMacro(Self);
    typedef std::map<std::string, TActionFunctor*> ActionFunctionsMapType;
    typedef itk::SmartPointer<StateMachineState> StateMachineStateType;

    /**
      * @brief Loads XML resource
      *
      * Loads a XML resource file in the given module context.
      * Default is the Mitk module (core).
      * The files have to be placed in the Resources/Interaction folder of their respective module.
      **/
    bool LoadStateMachine(std::string filename, std::string moduleName="Mitk");
    /**
     * Receives Event from Dispatcher.
     * Event is mapped using the EventConfig Object to a variant, then it is checked if the StateMachine is listening for
     * such an Event. If this is the case, the transition to the next state it performed and all actions associated with the transition executed,
     * and true is returned to the caller.
     * If the StateMachine can't handle this event false is returned.
     * Attention:
     * If a transition is associated with multiple actions - "true" is returned if one action returns true,
     * and the event is treated as HANDLED even though some actions might not have been executed! So be sure that all actions that occur within
     * one transitions have the same conditions.
     */
    bool HandleEvent(InteractionEvent* event, DataNode* dataNode);

  protected:
    EventStateMachine();
    virtual ~EventStateMachine();
    /**
     * Connects action from StateMachine (String in XML file) with a function that is called when this action is to be executed.
     */
    void AddActionFunction(std::string action, TActionFunctor* functor);

    StateMachineState* GetCurrentState();

    /**
     * Is called after loading a statemachine.
     * Overwrite this function in specific interactor implementations.
     * Connect actions and functions using the  CONNECT_FUNCTION macro within this function.
     */

    virtual void ConnectActionsAndFunctions();

    /**
     * Looks up function that is associated with action and executes it.
     * To implement your own execution scheme overwrite this in your DataInteractor.
     */
    virtual bool ExecuteAction(StateMachineAction* action, InteractionEvent* interactionEvent);

    /**
     * Implements filter scheme for events.
     * Standard implementation accepts events from 2d and 3d windows,
     * and rejects events if DataNode is not visible.
     * \return true if event is accepted, else false
     *
     * Overwrite this function to adapt for your own needs, for example to filter out events from
     * 3d windows like this:
     \code
      bool mitk::EventStateMachine::FilterEvents(InteractionEvent* interactionEvent, DataNode*dataNode)
      {
        return interactionEvent->GetSender()->GetMapperID() == BaseRenderer::Standard2D; // only 2D mappers
      }
     \endcode
     * or to enforce that the interactor only reacts when the corresponding DataNode is selected in the DataManager view..
     */
    virtual bool FilterEvents(InteractionEvent* interactionEvent, DataNode* dataNode);


  private:
    StateMachineContainer* m_StateMachineContainer; // storage of all states, action, transitions on which the statemachine operates.
    ActionFunctionsMapType m_ActionFunctionsMap; // stores association between action string
    StateMachineStateType m_CurrentState;
  };

} /* namespace mitk */
#endif /* MITKEVENTSTATEMACHINE_H_ */
