#ifndef MITKPOINTOPERATION
#define MITKPOINTOPERATION

#include "mitkCommon.h"
#include "Operation.h"
#include "mitkVector.h"


namespace mitk {

//##Documentation
//## @brief Operation, that handles all aktions on one Point.
//##
//## @ingroup Undo
//##
//## Tores everything for Adding, Moving and Deleting a Point.
//##
//## @params 
//## operationType is the type of that operation (see Operation.h; e.g. move or add; Information for StateMachine::ExecuteOperation())
//## point is the information of the point to add. or for the point to move into (change)
class PointOperation : public mitk::Operation
{
  public:
	//##Documentation

    PointOperation(OperationType operationType, 
					ITKPoint3D point, int index);
    
    virtual ~PointOperation();
	
	ITKPoint3D GetPoint();
	
	int GetIndex();

  private:
	ITKPoint3D m_Point;
	int m_Index;
};
}//namespace mitk
#endif /* MITKPOINTOPERATION*/