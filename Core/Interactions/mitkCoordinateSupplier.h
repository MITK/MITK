#ifndef MITKCOORDINATESUPPLIER_H
#define MITKCOORDINATESUPPLIER_H

#include "mitkCommon.h"
#include "StateMachine.h"
#include "mitkVector.h"

namespace mitk {

class Operation;
class OperationActor;

//##Documentation
//## @brief Interactor
//## 
//## sends a Point, that can be processed in its own OperationActor
class CoordinateSupplier : public StateMachine
{
public:
    mitkClassMacro(CoordinateSupplier, StateMachine);

	//##Documentation
	CoordinateSupplier(std::string type, OperationActor* operationActor);

protected:
	//##Documentation
	//## @brief 
	//##
	virtual bool ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int groupEventId, int objectEventId);

private:
	ITKPoint3D m_Point;
    OperationActor* m_Destination;
};

} // namespace mitk

#endif /* MITKCOORDINATESUPPLIER_H */
