#ifndef STATEMACHINE_H_HEADER_INCLUDED_C18896BD
#define STATEMACHINE_H_HEADER_INCLUDED_C18896BD

#include "mitkCommon.h"
#include <itkObject.h>
#include "mitkOperationActor.h"
#include "mitkState.h"
#include "mitkStateEvent.h"
#include "mitkUndoModel.h"
#include "mitkUndoController.h"
#include <string>


namespace mitk {

//##ModelId=3E5A397B01D5
//##Documentation
//## @brief superior statemachine
//## @ingroup Interaction
//## realizes the methods, that every statemachine has to have.
//## Undo can be enabled and disabled through EnableUndo
//## Developers must derive its statemachines and implement ExecuteSideEffect
//## and ExecuteOperation
	class StateMachine : public itk::Object, public mitk::OperationActor
{
  public:
  mitkClassMacro(StateMachine,itk::Object);

  //##ModelId=3E5B2DB301FD
  //##Documentation
  //## Constructor
  //## connects the current State to a
  //## StateMachine of Type type;
  //## default of m_UndoEnabled is true;
  StateMachine(std::string type);

  ~StateMachine(){}

  //##ModelId=3E5B2E660087
  std::string GetType() const;

  //##ModelId=3E5B2DE30378
  //##Documentation
  //## @brief handles an Event accordingly to its current State
  //##
  //## statechange with Undo functionality;
  //## groupEventId and objectEventId are use to combine Operations so that
  //## they can be undone together or seperately.
  //## EventMapper gives each event a new objectEventId
  //## and a StateMachine::ExecuteSideEffect can descide weather it gets a
  //## new GroupEventId or not, depending on its state (e.g. finishedNewObject then new GroupEventId)
  bool HandleEvent(StateEvent const* stateEvent, int objectEventId, int groupEventId);

  //##ModelId=3EDCAECB0175
  //##Documentation
  //## if set to true, then UndoFunctionality is enabled
  //## if false, then Undo is disabled
  void EnableUndo(bool enable);

  //##Documentation
  //## so that UndoModel can call ExecuteOperation for Undo!
  friend class UndoModel;

 protected:

  //##ModelId=3E5B2E170228
  //##Documentation
  //## each statechange has a sideeffect, which can be assigned by it's number
  //## if you are developing a new statemachine, you put all your operations here!
  //## dependant on the SideEffectNumber / build of the StateMachine-Definition
  //## First Undo, then SideEffectOperation
  virtual bool ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)= 0;

  //##ModelId=3EDCAECB00B9
  //##Documentation
  //## if true, then UndoFunctionality is enabled
  //## default on true;
  bool m_UndoEnabled;

  //Documentation
  //##ModelId=3EF099EA03C0
  //## @brief friend protected function of OperationEvent, that way all StateMachines can increment!
  int IncCurrGroupEventId();

  //##ModelId=3EDCAECB0128
  //##Documentation
  //## holds an UndoController, that can be accessed from all StateMachines. For ExecutreSideEffect
  UndoController* m_UndoController;

 private:
  //##ModelId=3EAEEDC603D9
  //##Documentation
  //## from OperationActor; a stateMachine has to be an OperationActor
  //## due to the UndoMechanism. Else no destination for Undo/Redo of StateMachines
  //## can be set.
  //## is set private here and in superclass it is set public, so UndoController
  //## can reach ist, but it can't be overwritten by a subclass
  void ExecuteOperation(Operation* operation);

  //##ModelId=3E5B2D66027E
  std::string m_Type;

  //##ModelId=3E5B2D8F02B9
  State* m_CurrentState;
 };

 } // namespace mitk



 #endif /* STATEMACHINE_H_HEADER_INCLUDED_C18896BD */

