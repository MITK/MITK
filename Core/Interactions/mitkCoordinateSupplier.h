#ifndef MITKCOORDINATESUPPLIER_H
#define MITKCOORDINATESUPPLIER_H

#include "mitkCommon.h"
#include "StateMachine.h"
#include "mitkVector.h"

namespace mitk {

class Operation;
class OperationActor;

//##ModelId=3F0189F00094
//##Documentation
//## @brief Interactor
//## 
//## sends a Point, that can be processed in its own OperationActor
class CoordinateSupplier : public StateMachine
{
public:
    //##ModelId=3F0189F0024E
    mitkClassMacro(CoordinateSupplier, StateMachine);

    //##ModelId=3F0189F0025B
	//##Documentation
	CoordinateSupplier(std::string type, OperationActor* operationActor);

protected:
    //##ModelId=3F0189F00269
	//##Documentation
	//## @brief 
	//##
	virtual bool ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int groupEventId, int objectEventId);

private:
    //##ModelId=3F0189F0023B
	ITKPoint3D m_Point;
    //##ModelId=3F0189F0024A
    OperationActor* m_Destination;
};

} // namespace mitk

#endif /* MITKCOORDINATESUPPLIER_H */
