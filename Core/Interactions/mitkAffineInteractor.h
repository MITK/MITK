#ifndef MITKAFINEINTERACTOR_H_HEADER_INCLUDED_C188C29F
#define MITKAFINEINTERACTOR_H_HEADER_INCLUDED_C188C29F

#include "mitkCommon.h"
#include "mitkInteractor.h"

namespace mitk {

//##Documentation
//## @brief Interactor for afine transformations translate, rotate 
//## @ingroup Interaction
//## 
//## An object of this class can translate, rotate and scale the Data
class AfineInteractor : public Interactor
{
public:
  mitkClassMacro(AfineInteractor,Interactor);

  itkNewMacro(Self);  
	//##Documentation
	//## @brief Constructor
	//##
	//## @params dataTreeNode is the node, this Interactor is connected to
	//## type is the type of StateMachine like declared in the XML-Configure-File
	//##

protected:
 	AfineInteractor(std::string type, DataTreeNode* dataTreeNode);
  AfineInteractor(){};
  ~AfineInteractor(){};

	virtual bool ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId);
};

} // namespace mitk



#endif /* MITKAFINEINTERACTOR_H_HEADER_INCLUDED_C188C29F */
