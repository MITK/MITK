#ifndef MITKAFFINEINTERACTOR_H_HEADER_INCLUDED_C188C29F
#define MITKAFFINEINTERACTOR_H_HEADER_INCLUDED_C188C29F

#include "mitkCommon.h"
#include "mitkInteractor.h"
#include "mitkVector.h"

namespace mitk {

//##Documentation
//## @brief Interactor for Affine transformations translate, rotate 
//## @ingroup Interaction
//## 
//## An object of this class can translate, rotate and scale the Data
class AffineInteractor : public Interactor
{
public:
  mitkClassMacro(AffineInteractor,Interactor);

	//##Documentation
	//## @brief Constructor
	//##
	//## @params dataTreeNode is the node, this Interactor is connected to
	//## type is the type of StateMachine like declared in the XML-Configure-File
	//##
 	AffineInteractor(std::string type, DataTreeNode* dataTreeNode);

  //##Documentation
	//## @brief Destructor
  ~AffineInteractor(){};

protected:

	virtual bool ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId);

  mitk::ITKPoint3D m_lastScalePosition;
  mitk::ITKPoint3D m_lastScaleData;
  mitk::ITKPoint3D m_lastRotatePosition;
  mitk::ITKPoint3D m_lastRotateData;
  float m_ScaleFactor;
};

} // namespace mitk



#endif /* MITKAFFINEINTERACTOR_H_HEADER_INCLUDED_C188C29F */
