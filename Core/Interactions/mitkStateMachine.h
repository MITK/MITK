/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef STATEMACHINE_H_HEADER_INCLUDED_C18896BD
#define STATEMACHINE_H_HEADER_INCLUDED_C18896BD

#include "mitkCommon.h"
#include <itkObject.h>
#include "mitkOperationActor.h"
#include <string>

#include "mitkUndoModel.h"
#include <mitkXMLIO.h>

namespace mitk {

  class State;
  class Action;
  class StateEvent;
  class UndoController;
  
  // base class of statem machine functors
  class TStateMachineFunctor
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


//##ModelId=3E5A397B01D5
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
  class StateMachine : public itk::Object, public mitk::OperationActor, public XMLIO
  {

  public:
    mitkClassMacro(StateMachine,itk::Object);
   
    /// map to connect action IDs with method calls
    /// Use AddActionFunction or (even better) the CONNECT_ACTION macro to fill the map
    typedef std::map<int, TStateMachineFunctor*> ActionFunctionsMapType;

    //##ModelId=3E5B2DB301FD
    //##Documentation
    //## Constructor
    //## @brief connects the current State to a StateMachine of Type type;
    //## 
    //## Because of the use of Smartpointers in DataTree and the use of observer-technologies, a StateMachine is a Subclass of itk::Object.
    //## default of m_UndoEnabled is true;
    StateMachine(const char * type);

    ~StateMachine();

    //##ModelId=3E5B2E660087
    std::string GetType() const;

    //##ModelId=3E5B2DE30378
    //##Documentation
    //## @brief handles an Event accordingly to its current State
    //##
    //## statechange with Undo functionality;
    //## EventMapper gives each event a new objectEventId
    //## and a StateMachine::ExecuteAction can descide weather it gets a
    //## new GroupEventId or not, depending on its state (e.g. finishedNewObject then new GroupEventId).
    //## Object- and group-EventId can also be accessed through static methods from OperationEvent
    virtual bool HandleEvent(StateEvent const* stateEvent);

    //##ModelId=3EDCAECB0175
    //##Documentation
    //## if set to true, then UndoFunctionality is enabled
    //## if false, then Undo is disabled
    void EnableUndo(bool enable);

    //##Documentation
    //## so that UndoModel can call ExecuteOperation for Undo!
    friend class UndoModel;

    //##
    virtual bool WriteXMLData( XMLWriter& xmlWriter );
    //##
    virtual bool ReadXMLData( XMLReader& xmlReader );

    static const std::string XML_NODE_NAME;

  protected:

    void AddActionFunction(int action, TStateMachineFunctor* functor);

    //##ModelId=3E5B2E170228
    //##Documentation
    //## @brief Method called in HandleEvent after Statechange.
    //##
    //## Each statechange has actions, which can be assigned by it's number.
    //## If you are developing a new statemachine, declare all your operations here and send them to Undo-Controller and to the Data.
    //## Object- and group-EventId can also be accessed through static methods from OperationEvent
    virtual bool ExecuteAction(Action* action, StateEvent const* stateEvent);

    //##Documentation
    //## @brief returns the current state
    const State* GetCurrentState() const;

    //##ModelId=3EDCAECB00B9
    //##Documentation
    //## @brief if true, then UndoFunctionality is enabled
    //## default on true;
    bool m_UndoEnabled;

    //Documentation
    //##ModelId=3EF099EA03C0
    //## @brief friend protected function of OperationEvent, that way all StateMachines can set GroupEventId to be incremented!
    void IncCurrGroupEventId();

    //##ModelId=3EDCAECB0128
    //##Documentation
    //## @brief holds an UndoController, that can be accessed from all StateMachines. For ExecuteAction
    UndoController* m_UndoController;

    //##ModelId=3EAEEDC603D9
    //##Documentation
    //## @brief A statemachine is also an OperationActor due to the UndoMechanism. 
    //## 
    //## The statechange is done in ExecuteOperation, so that the statechange can be undone by UndoMechanism.
    //## Is set private here and in superclass it is set public, so UndoController
    //## can reach ist, but it can't be overwritten by a subclass
    virtual void ExecuteOperation(Operation* operation);

    //##Documentation
    //## @brief resets the current state to the startstate with undo functionality! Use carefully!
    void ResetStatemachineToStartState();

    //##Documentation
    // XML
    virtual const std::string& GetXMLNodeName() const;

    static const std::string STATE_MACHINE_TYPE;
    static const std::string STATE_ID;

  private:

    //##ModelId=3E5B2D66027E
    std::string m_Type;

    //##ModelId=3E5B2D8F02B9
    //##Documentation
    //## @brief holds the current state the machine is in
    State* m_CurrentState;
    
    ActionFunctionsMapType m_ActionFunctionsMap;

  };

} // namespace mitk



#endif /* STATEMACHINE_H_HEADER_INCLUDED_C18896BD */

