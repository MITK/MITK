#ifndef MITKCOORDINATESUPPLIER_H
#define MITKCOORDINATESUPPLIER_H

#include "mitkCommon.h"
#include "mitkStateMachine.h"
#include "mitkVector.h"

namespace mitk {

class Operation;
class OperationActor;

//##ModelId=3F0189F00094
//##Documentation
//## @brief Interactor
//## @ingroup Interaction
//## sends a Point, that can be processed in its own OperationActor
class CoordinateSupplier : public StateMachine
{
public:
    //##ModelId=3F0189F0024E
    mitkClassMacro(CoordinateSupplier, StateMachine);

    //##ModelId=3F0189F0025B
	//##Documentation
    //## @brief Constructor with needed arguments
    //## @params: type: string, that describes the StateMachine-Scheme to take from all SM (see XML-File)
    //##    operationActor: the Data, operations (+ points) are send to
	CoordinateSupplier(const char * type, OperationActor* operationActor);

protected:
    //##ModelId=3F0189F00269
	//##Documentation
	//## @brief 
	//##
	virtual bool ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId);

private:
  //##ModelId=3F0189F0024A
  OperationActor* m_Destination;
  ITKPoint3D m_OldPoint;

};

} // namespace mitk

#endif /* MITKCOORDINATESUPPLIER_H */
