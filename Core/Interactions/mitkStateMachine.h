#ifndef STATEMACHINE_H_HEADER_INCLUDED_C18896BD
#define STATEMACHINE_H_HEADER_INCLUDED_C18896BD

#include "mitkCommon.h"
#include <itkObject.h>
#include "OperationActor.h"
#include "State.h"
#include "StateEvent.h"
#include <string>


namespace mitk {

//##ModelId=3E5A397B01D5
	class StateMachine : public itk::Object, public mitk::OperationActor
{
  public:
    //##ModelId=3E5B2DB301FD
	//##Documentation
    //## Constuctor; string type is used as a name for that StateMachine
	StateMachine(std::string type);

    //##ModelId=3E5B2E660087
	std::string GetType() const;

    //##ModelId=3E5B2DE30378
	//##Documentation
    //## gets StateEvent from EventMapper and handles it
	//## according to stateEvent->GetId()
	//## statechange with Undo functionality
    bool HandleEvent(StateEvent const* stateEvent);

  protected:
    //##ModelId=3E5B2E170228
    virtual bool ExecuteSideEffect(int sideEffectId)= 0;

  private:  
	//##ModelId=3E5B2D66027E
	std::string m_Type;

    //##ModelId=3E5B2D8F02B9
    State* m_CurrentState;
};

} // namespace mitk



#endif /* STATEMACHINE_H_HEADER_INCLUDED_C18896BD */
