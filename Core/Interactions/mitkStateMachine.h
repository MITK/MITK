#ifndef STATEMACHINE_H_HEADER_INCLUDED_C18896BD
#define STATEMACHINE_H_HEADER_INCLUDED_C18896BD

#include "mitkCommon.h"
#include <itkObject.h>
#include "State.h"
#include "StateEvent.h"
#include <string>


namespace mitk {

//##ModelId=3E5A397B01D5
class StateMachine : public itk::Object
{
  public:
    //##ModelId=3E5B2DB301FD
	StateMachine(std::string type);

    //##ModelId=3E5B2DE30378
    bool HandleEvent(StateEvent const* stateEvent);

    //##ModelId=3E5B2E660087
	std::string GetName() const;

  protected:
    //##ModelId=3E5B2E170228
    virtual bool ExecuteSideEffect(int sideEffectId);

  private:
    //##ModelId=3E5B2D66027E
	std::string m_Type;

    //##ModelId=3E5B2D8F02B9
    State* m_CurrentState;

};

} // namespace mitk



#endif /* STATEMACHINE_H_HEADER_INCLUDED_C18896BD */
