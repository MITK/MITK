#ifndef STATEMACHINE_H_HEADER_INCLUDED_C18896BD
#define STATEMACHINE_H_HEADER_INCLUDED_C18896BD

#include "mitkCommon.h"
#include <itkObject.h>
#include "OperationActor.h"
#include "State.h"
#include "StateEvent.h"
#include "UndoController.h"

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
    //##ModelId=3E5B2DB301FD
	//##Documentation
	//## Constructor
	//## connects the current State to a
	//## StateMachine of Type type;
    //## default of m_UndoEnabled is true;
	StateMachine(std::string type);

    //##ModelId=3E5B2E660087
	std::string GetType() const;

    //##ModelId=3E5B2DE30378
	//##Documentation
    //## gets StateEvent from EventMapper and handles it
	//## according to stateEvent->GetId()
	//## statechange with Undo functionality
    bool HandleEvent(StateEvent const* stateEvent);

    //##ModelId=3EAEEDC603D9
	//##Documentation
    //## from OperationActor; a stateMachine has to be an OperationActor
	//## due to the UndoMechanism. Else no destination for Undo/Redo of StateMachines
	//## can be set.
	virtual void ExecuteOperation(Operation* operation) = 0;

    //##ModelId=3EDCAECB0175
	//##Documentation
    //## if set to true, then UndoFunctionality is enabled
	//## if false, then Undo is disabled
	void EnableUndo(bool enable);

  protected:
    //##ModelId=3E5B2E170228
	//##Documentation
    //## each statechange has a sideeffect, which can be assigned by it's number
    //## if you are developing a new statemachine, you put all your operations here!
    //## dependant on the SideEffectNumber / build of the StateMachine-Definition
	//## First Undo, then SideEffectOperation
    virtual bool ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent)= 0;

	//##ModelId=3EDCAECB00B9
	//##Documentation
	//## if true, then UndoFunctionality is enabled
	//## default on true;
	bool m_UndoEnabled;

  private:
	//##ModelId=3E5B2D66027E
	std::string m_Type;

    //##ModelId=3E5B2D8F02B9
    State* m_CurrentState;

    

  protected:
    //##ModelId=3EDCAECB0128
	//##Documentation
	//## holds an UndoController, that can be accessed from all StateMachines
	UndoController* m_UndoController;


};

} // namespace mitk



#endif /* STATEMACHINE_H_HEADER_INCLUDED_C18896BD */
