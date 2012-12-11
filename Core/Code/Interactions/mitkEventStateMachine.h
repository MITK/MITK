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

  /**
   * Base class of ActionFunctors, to provide an easy to connect actions with functions.
   */

  class MITK_CORE_EXPORT TActionFunctor
  {
  public:
    virtual bool DoAction(StateMachineAction*, const InteractionEvent*)=0;
    virtual ~TActionFunctor()
    {
    }
  };

  /**
   * Specific implementation of ActionFunctor class, implements a reference to the function which is to be executed. It takes two arguments:
   * StateMachineAction - the action by which the function call is invoked, InteractionEvent - the event that caused the transition.
   */
  template<class T>
  class TSpecificActionFunctor: public TActionFunctor
  {
  public:

    TSpecificActionFunctor(T* object, bool (T::*memberFunctionPointer)(StateMachineAction*, const InteractionEvent*)) :
        m_Object(object), m_MemberFunctionPointer(memberFunctionPointer)
    {
    }

    virtual ~TSpecificActionFunctor()
    {
    }
    virtual bool DoAction(StateMachineAction* action, const InteractionEvent* event)
    {
      return (*m_Object.*m_MemberFunctionPointer)(action, event);// executes member function
    }

  private:
    T* m_Object;
    bool (T::*m_MemberFunctionPointer)(StateMachineAction*, const InteractionEvent*);
  };


  /** Macro that can be used to connect a StateMachineAction with a function.
   *  It assumes that there is a typedef Classname Self in classes that use this macro, as is provided by e.g. mitkClassMacro
   */
#define CONNECT_FUNCTION(a, f) \
    EventStateMachine::AddActionFunction(a, new TSpecificStateMachineFunctor<Self>(this, &Self::f));

  /**
   * Super-class that provides the functionality of a StateMachine for DataInteractors.
   */

  class MITK_CORE_EXPORT EventStateMachine: public EventHandler
  {

  public:
    mitkClassMacro(EventStateMachine, EventHandler);
    itkNewMacro(Self);
    typedef std::map<std::string, TActionFunctor*> ActionFunctionsMapType;
    typedef itk::SmartPointer<StateMachineState> StateMachineStateType;

    void LoadStateMachine(std::string filename);
    /**
     * Receives Event from Dispatcher.
     * Event is mapped using the EventConfig Object to a variant, then it is checked if the StateMachine is listening for
     * such an Event. If this is the case, the transition to the next state it performed and all actions associated with the transition executed,
     * and true is returned to the caller.
     * If the StateMachine can't handle this event false is returned.
     * Attention:
     * If a transition is associated with multiple actions - "false" is returned if ALL action return false,
     * and the event is treated as HANDLED even though some actions might not have been executed! So be sure that all actions that occur within
     * one transitions have the same conditions.
     */
    bool HandleEvent(InteractionEvent* event);

  protected:
    EventStateMachine();
    virtual ~EventStateMachine();
    /**
     * Connects action from StateMachine (String in XML file) with a function that is called when this action is to be executed.
     */
    void AddActionFunction(std::string action, TActionFunctor* functor);

    /**
     * Looks up function that is associated with action and executes it.
     * To implement your own execution scheme overwrite this in your DataInteractor.
     */
    virtual bool ExecuteAction(StateMachineAction* action, InteractionEvent* stateEvent);


  private:
    StateMachineContainer* m_StateMachineContainer; // storage of all states, action, transitions on which the statemachine operates.
    ActionFunctionsMapType m_ActionFunctionsMap; // stores association between action string
    StateMachineStateType m_CurrentState;
  };

} /* namespace mitk */
#endif /* MITKEVENTSTATEMACHINE_H_ */
