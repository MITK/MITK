#ifndef STATETRANSITIONOPERATION_H_HEADER_INCLUDED_C1695462
#define STATETRANSITIONOPERATION_H_HEADER_INCLUDED_C1695462

#include "mitkCommon.h"
#include "Operation.h"
#include "State.h"

namespace mitk {
//##ModelId=3E782FC201BB
//##Documentation
//## operation, that changes the stateMachine and sets a state according to a
//## transition to an other state.
class StateTransitionOperation : public mitk::Operation
{
  public:
	  //##ModelId=3E78311303E7
      StateTransitionOperation(OperationType operationType, int execId, State* state);

	  //##ModelId=3E78307601D8
	  State* GetState();

    //##ModelId=3E96C2EE00DD
	  virtual void Execute();

      //##ModelId=3EDDD7640192
	  virtual void Execute(int eventId);//wrong?!?
    //##ModelId=3EDDD7640154
	  //##Documentation
	  //##Constants for Operation::m_ExecutionId
	  static const int NOTHING;
    //##ModelId=3EDDD7640163
	  static const int CHANGESTATE;

  private:
    //##ModelId=3E78305B00D5
    State* m_State;

};
}//namespace mitk
#endif /* STATETRANSITIONOPERATION_H_HEADER_INCLUDED_C1695462 */
