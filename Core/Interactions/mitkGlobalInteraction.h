#ifndef GLOBALINTERACTION_H_HEADER_INCLUDED_C152938A
#define GLOBALINTERACTION_H_HEADER_INCLUDED_C152938A

#include "mitkCommon.h"
#include "StateMachine.h"
#include "Focus.h"
//#include "DataTree.h"
#include <string>
#include <vector>


namespace mitk {

//class Group;

//##ModelId=3E5B33000230
class GlobalInteraction : public StateMachine
{
	public:
	//##ModelId=3EAD420E0088
	GlobalInteraction(std::string type);

    //##ModelId=3E7F497F01AE
    virtual bool ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent);

    //##ModelId=3EDB725E0146
	virtual void ExecuteOperation(mitk::Operation* operation);

	private:
	//##ModelId=3EAD4EF903A5
//	std::vector<Roi*> m_SelectedElements;//ERROR!!!

    //##ModelId=3EAD4F3A03C6
    Focus* m_Focus;

    //##ModelId=3EAD4F51010C
    //Group* m_Group;

    //##ModelId=3EAD502B01EC
 //   DataTree* m_DataTree;

};


} // namespace mitk



#endif /* GLOBALINTERACTION_H_HEADER_INCLUDED_C152938A */
