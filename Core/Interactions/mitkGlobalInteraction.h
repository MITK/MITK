#ifndef GLOBALINTERACTION_H_HEADER_INCLUDED_C152938A
#define GLOBALINTERACTION_H_HEADER_INCLUDED_C152938A

#include "Focus.h"
#include "mitkCommon.h"
#include "StateMachine.h"
#include "Roi.h"
//#include "DataTree.h"
#include <string>
#include <vector>


namespace mitk {

//class Group;

//##ModelId=3E5B33000230
//##Documentation
//## @brief handles all global Events ("not specified on one object"-Events).
//## @ingroup Interaction
//## 
//## superior statemachine, that handles the events and asks all other ROI's
//## through the information of RoiFactory to process the event.
class GlobalInteraction : public StateMachine
{
	public:
	//##ModelId=3EAD420E0088
	GlobalInteraction(std::string type);

    //##ModelId=3E7F497F01AE
    virtual bool ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent);

    //##ModelId=3EDCAECA0194
	virtual void ExecuteOperation(mitk::Operation* operation);

    //##ModelId=3EDDD76302BB
	//##Documentation
	//## quickimplementation!!!
	//## method is to be excluded from GlobalInteraction
	void SetRoi(Roi* roi);

	private:
	//##ModelId=3EAD4EF903A5
//	std::vector<Roi*> m_SelectedElements;//ERROR!!!

    //##ModelId=3EAD4F3A03C6
    Focus* m_Focus;

    //##ModelId=3EDDD763029D
    //Group* m_Group;

    //##ModelId=3EAD502B01EC
//   DataTree* m_DataTree;

	//##Documentation
	//## quickimplementation! Thought is a list of StateMachines to be asked.
	//## take the information from BaseRenderer and parse through the tree and save all interactables
	//## if Tree changes, then build up a new list.
	//## if a new object is added to the tree, then pudh the old one in Undo and set the new 
	//## to the current (m_selectedElements)...
	Roi* m_Roi;


};
} // namespace mitk

#endif /* GLOBALINTERACTION_H_HEADER_INCLUDED_C152938A */
