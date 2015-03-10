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


#ifndef STATEMACHINE_H_HEADER_INCLUDED_C18896BD
#define STATEMACHINE_H_HEADER_INCLUDED_C18896BD

#include <MitkCoreExports.h>
#include <itkObject.h>
#include "mitkOperationActor.h"
#include <string>
#include "mitkState.h"
#include "mitkUndoModel.h"

namespace mitk {

  class Action;
  class StateEvent;
  class UndoController;

  // base class of statem machine functors

  class MITKCORE_EXPORT TStateMachineFunctor
  {
    public:
      virtual bool DoAction(Action*, const StateEvent*)=0;        // call using function
      virtual ~TStateMachineFunctor() {}
  };

  // the template functor for arbitrary StateMachine derivations
  template <class T>
  class TSpecificStateMachineFunctor : public TStateMachineFunctor
  {
    public:

      // constructor - takes pointer to an object and pointer to a member and stores
      // them in two private variables
      TSpecificStateMachineFunctor(T* object, bool(T::*memberFunctionPointer)(Action*, const StateEvent*))
      :m_Object(object),
      m_MemberFunctionPointer(memberFunctionPointer)
      {
      }

      virtual ~TSpecificStateMachineFunctor() {} // virtual destructor

      // override function "Call"
      virtual bool DoAction(Action* action, const StateEvent* stateEvent)
      {
        return (*m_Object.*m_MemberFunctionPointer)(action, stateEvent);             // execute member function
      }

    private:
      T* m_Object;                  // pointer to object
      bool (T::*m_MemberFunctionPointer)(Action*, const StateEvent*);   // pointer to member function
  };

/// Can be uses by derived classes of StateMachine to connect action IDs to methods
/// Assumes that there is a typedef Classname Self in classes that use this macro
#define CONNECT_ACTION(a, f) \
  StateMachine::AddActionFunction(a, new TSpecificStateMachineFunctor<Self>(this, &Self::f));

#define STATEMACHINE_INFO MITK_INFO("StateMachine") << "[type: " << GetType() << "] "
#define STATEMACHINE_WARN MITK_WARN("StateMachine") << "[type: " << GetType() << "] "
#define STATEMACHINE_FATAL MITK_FATAL("StateMachine") << "[type: " << GetType() << "] "
#define STATEMACHINE_ERROR MITK_ERROR("StateMachine") << "[type: " << GetType() << "] "
#define STATEMACHINE_DEBUG MITK_DEBUG("StateMachine") << "[type: " << GetType() << "] "

/**
@brief Superior statemachine
@ingroup Interaction

  Realizes the methods, that every statemachine has to have.
  Undo can be enabled and disabled through EnableUndo.

  To implement your own state machine, you have to derive a class from mitk::StateMachine and either

  - override ExecuteAction()
  or
  - Write bool methods that take (Action*, const StateEvent*) as parameter and use the CONNECT_ACTION macro in your constructor

  The second version is recommended, since it provides more structured code. The following piece of code demonstrates how to
  use the CONNECT_ACTION macro. The important detail is to provide a <i>typedef classname Self</i>

  \code
class LightSwitch : public StateMachine
{
  public:

    mitkClassMacro(LightSwitch, StateMachine); // this creates the Self typedef

    LightSwitch(const char*);

    bool DoSwitchOn(Action*, const StateEvent*);
    bool DoSwitchOff(Action*, const StateEvent*);
}

LightSwitch::LightSwitch(const char* type)
:StateMachine(type)
{
  // make sure that AcSWITCHON and AcSWITCHOFF are defined int constants somewhere (e.g. mitkInteractionConst.h)
  CONNECT_ACTION( AcSWITCHON, DoSwitchOn );
  CONNECT_ACTION( AcSWITCHOFF, DoSwitchOff );
}

bool LightSwitch::DoSwitchOn(Action*, const StateEvent*)
{
  std::cout << "Enlightenment" << std::endl;
}

bool LightSwitch::DoSwitchOff(Action*, const StateEvent*)
{
  std::cout << "Confusion" << std::endl;
}
\endcode

  What CONNECT_ACTION does, is call StateMachine::AddActionFunction(...) to add some function pointer wrapping class (functor) to
  a std::map of StateMachine. Whenever StateMachines ExecuteAction is called, StateMachine will lookup the desired Action in its
  map and call the appropriate method in your derived class.

**/
  /**
    * \deprecatedSince{2013_03} mitk::StateMachine is deprecated. Use mitk::EventStateMachine instead.
    * Refer to \see DataInteractionPage for general information about the concept of the new implementation.
  */

  class MITKCORE_EXPORT StateMachine : public itk::Object, public mitk::OperationActor
  {

  public:
    mitkClassMacro(StateMachine,itk::Object);

    /**
    * @brief New Macro with one parameter for creating this object with static New(..) method
    **/
    mitkNewMacro1Param(Self, const char*);


    /**
    * @brief Map to connect action IDs with method calls. Use AddActionFunction or (even better) the CONNECT_ACTION macro to fill the map.
    **/
    typedef std::map<int, TStateMachineFunctor*> ActionFunctionsMapType;

    /**
    * @brief Type for a vector of StartStatePointers
    **/
    typedef std::vector<State::Pointer> StartStateVectorType;

    /**
    * @brief Get the name and with this the type of the StateMachine
    **/
    std::string GetType() const;

    /**
    * @brief handles an Event accordingly to its current State
    *
    * Statechange with Undo functionality;
    * EventMapper gives each event a new objectEventId
    * and a StateMachine::ExecuteAction can descide weather it gets a
    * new GroupEventId or not, depending on its state (e.g. finishedNewObject then new GroupEventId).
    * Object- and group-EventId can also be accessed through static methods from OperationEvent
    **/
    virtual bool HandleEvent(StateEvent const* stateEvent);

    /**
     * @brief calculates how good this statemachine can handle the event.
     *
     *  Returns a value between 0 and 1
     *  where 0 represents not responsible and 1 represents definitive responsible!
     *  Standard function to override if needed.
     *  (Used by GlobalInteraction to decide which DESELECTED statemachine to send the event to.)
    **/
    virtual float CanHandleEvent(const StateEvent *) const;

    /**
    * @brief Enables or disabled Undo.
    **/
    void EnableUndo(bool enable);

    /**
    * @brief A statemachine is also an OperationActor due to the UndoMechanism.
    *
    * The statechange is done in ExecuteOperation, so that the statechange can be undone by UndoMechanism.
    * Is set private here and in superclass it is set public, so UndoController
    * can reach ist, but it can't be overwritten by a subclass
    * *ATTENTION*: THIS METHOD SHOULD NOT BE CALLED FROM OTHER CLASSES DIRECTLY!
    **/
    virtual void ExecuteOperation(Operation* operation);

    /**
    * @brief Friend so that UndoModel can call ExecuteOperation for Undo.
    **/
    friend class UndoModel;

    friend class GlobalInteraction;


  protected:
    /**
    * @brief Default Constructor. Obsolete to instanciate it with this method! Use ::New(..) method instead. Set the "type" and with this the pattern of the StateMachine
    **/
    StateMachine(const char * type);

    /**
    * @brief Default Destructor
    **/
    ~StateMachine();

    /**
    * @brief Adds the Function to ActionList.
    **/
    void AddActionFunction(int action, TStateMachineFunctor* functor);

    /**
    * @brief Method called in HandleEvent after Statechange.
    *
    * Each statechange has actions, which can be assigned by it's number.
    * If you are developing a new statemachine, declare all your operations here and send them to Undo-Controller and to the Data.
    * Object- and group-EventId can also be accessed through static methods from OperationEvent
    **/
    virtual bool ExecuteAction(Action* action, StateEvent const* stateEvent);

    /**
    * @brief returns the current state
    **/
    const State* GetCurrentState(unsigned int timeStep = 0) const;

    /**
    * @brief if true, then UndoFunctionality is enabled
    *
    * Default value is true;
    **/
    bool m_UndoEnabled;

    /**
    * @brief Friend protected function of OperationEvent; that way all StateMachines can set GroupEventId to be incremented!
    **/
    void IncCurrGroupEventId();

    /**
    * @brief holds an UndoController, that can be accessed from all StateMachines. For ExecuteAction
    **/
    UndoController* m_UndoController;

    /**
    * @brief Resets the current state from the given timeStep to the StartState with undo functionality! Use carefully!
    * @param[in] timeStep If the statemachine has several timesteps to take care of, specify the according timestep
    **/
    void ResetStatemachineToStartState(unsigned int timeStep = 0);

    /**
    * @brief Check if the number of timeSteps is equal to the number of stored StartStates. Nothing is changed if the number is equal.
    **/
    void ExpandStartStateVector(unsigned int timeSteps);

    /**
    * @brief initializes m_CurrentStateVector
    **/
    void InitializeStartStates(unsigned int timeSteps);

    /**
    * @brief Update the TimeStep of the statemachine with undo-support if undo enabled
    **/
    virtual void UpdateTimeStep(unsigned int timeStep);

    /**
    * @brief Current TimeStep if the data which is to be interacted on, has more than 1 TimeStep
    **/
    unsigned int m_TimeStep;

  private:
    /**
    * @brief The type of the StateMachine. This string specifies the StateMachinePattern that is loaded from the StateMachineFactory.
    **/
    std::string m_Type;

    /**
    * @brief Points to the current state.
    **/
    StartStateVectorType m_CurrentStateVector;

    /**
    * @brief Map of the added Functions
    **/
    ActionFunctionsMapType m_ActionFunctionsMap;

  };

} // namespace mitk



#endif /* STATEMACHINE_H_HEADER_INCLUDED_C18896BD */


