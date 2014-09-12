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
#include "mitkCommon.h"
#include "mitkMessage.h"
#include "mitkInteractionEventHandler.h"

#include <MitkCoreExports.h>
#include <string>

/**
 * Macro that can be used to connect a StateMachineAction with a function.
 * It assumes that there is a typedef Classname Self in classes that use
 * this macro, as is provided by e.g. mitkClassMacro
 */
#define CONNECT_FUNCTION(a, f) \
  ::mitk::EventStateMachine::AddActionFunction(a, ::mitk::MessageDelegate2<Self, ::mitk::StateMachineAction*, ::mitk::InteractionEvent*, bool>(this, &Self::f));

#define CONNECT_CONDITION(a, f) \
  ::mitk::EventStateMachine::AddConditionFunction(a, ::mitk::MessageDelegate1<Self,const ::mitk::InteractionEvent*, bool>(this, &Self::f));


namespace us {
class Module;
}

namespace mitk
{
  class StateMachineTransition;
  class StateMachineContainer;
  class StateMachineAction;
  class StateMachineCondition;
  class InteractionEvent;
  class StateMachineState;
  class DataNode;
  class UndoController;

  /**
   * \class TActionFunctor
   * \brief Base class of ActionFunctors, to provide an easy to connect actions with functions.
   *
   * \deprecatedSince{2013_03} Use mitk::Message classes instead.
   */
  class TActionFunctor
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
  class DEPRECATED() TSpecificActionFunctor : public TActionFunctor
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
  class MITK_CORE_EXPORT EventStateMachine : public mitk::InteractionEventHandler
  {

  public:
    mitkClassMacro(EventStateMachine, InteractionEventHandler)

    typedef std::map<std::string, TActionFunctor*> DEPRECATED(ActionFunctionsMapType);

    typedef itk::SmartPointer<StateMachineState> StateMachineStateType;

    /**
      * @brief Loads XML resource
      *
      * Loads a XML resource file from the given module.
      * Default is the Mitk module (core).
      * The files have to be placed in the Resources/Interaction folder of their respective module.
      **/
    bool LoadStateMachine(const std::string& filename, const us::Module* module = NULL);
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

    /**
    * @brief Enables or disabled Undo.
    **/
    void EnableUndo(bool enable)
    {
      m_UndoEnabled = enable;
    }

    /**
    * @brief Enables/disables the state machine. In un-enabled state it won't react to any events.
    **/
    void EnableInteraction(bool enable)
    {
      m_IsActive = enable;
    }

  protected:
    EventStateMachine();
    virtual ~EventStateMachine();

    typedef MessageAbstractDelegate2<StateMachineAction*, InteractionEvent*, bool> ActionFunctionDelegate;
    typedef MessageAbstractDelegate1<const InteractionEvent*, bool> ConditionFunctionDelegate;

    /**
     * Connects action from StateMachine (String in XML file) with a function that is called when this action is to be executed.
     */
    DEPRECATED(void AddActionFunction(const std::string& action, TActionFunctor* functor));

    void AddActionFunction(const std::string& action, const ActionFunctionDelegate& delegate);

    void AddConditionFunction(const std::string& condition, const ConditionFunctionDelegate& delegate);

    StateMachineState* GetCurrentState() const;

    /**
     * @brief ResetToStartState Reset state machine to it initial starting state.
     */

    void ResetToStartState();

    /**
     * Is called after loading a statemachine.
     * Overwrite this function in specific interactor implementations.
     * Connect actions and functions using the  CONNECT_FUNCTION macro within this function.
     */

    virtual void ConnectActionsAndFunctions();

    virtual bool CheckCondition( const StateMachineCondition& condition, const InteractionEvent* interactionEvent );

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


    /**
    * \brief Returns the executable transition for the given event.
    *
    * This method takes a list of transitions that correspond to the given
    * event from the current state.
    *
    * This method iterates through all transitions and checks all
    * corresponding conditions. The results of each condition in stored in
    * map, as other transitions may need the same condition again.
    *
    * As soon as a transition is found for which all conditions are
    * fulfilled, this instance is returned.
    *
    * If a transition has no condition, it is automatically returned.
    * If no executable transition is found, NULL is returned.
    */
    StateMachineTransition* GetExecutableTransition( InteractionEvent* event );

    // Determines if state machine reacts to events
    bool m_IsActive;
    // Undo/Redo
    UndoController* m_UndoController;
    bool m_UndoEnabled;

  private:

    typedef std::map<std::string, ActionFunctionDelegate*> ActionDelegatesMapType;
    typedef std::map<std::string, ConditionFunctionDelegate*> ConditionDelegatesMapType;

    StateMachineContainer* m_StateMachineContainer; // storage of all states, action, transitions on which the statemachine operates.
    std::map<std::string, TActionFunctor*> m_ActionFunctionsMap; // stores association between action string
    ActionDelegatesMapType m_ActionDelegatesMap;
    ConditionDelegatesMapType m_ConditionDelegatesMap;
    StateMachineStateType m_CurrentState;


  };

} /* namespace mitk */
#endif /* MITKEVENTSTATEMACHINE_H_ */
