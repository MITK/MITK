#ifndef MITKPOINTOPERATION
#define MITKPOINTOPERATION

#include "mitkCommon.h"
#include "Operation.h"
#include "mitkVector.h"


namespace mitk {

//##ModelId=3F0189F0016F
//##Documentation
//## @brief Operation, that handles all aktions on one Point.
//##
//## @ingroup Undo
//##
//## Stores everything for Adding, Moving and Deleting a Point.
class PointOperation : public mitk::Operation
{
  public:
    //##ModelId=3F0189F003A2
	//##Documentation
	//##@brief Operation, that handles all aktions on one Point.
	//##
	//## @params
	//## operationType is the type of that operation (see Operation.h; e.g. move or add; Information for StateMachine::ExecuteOperation());
	//## point is the information of the point to add or is the information to change a point into; index is e.g. the position in a
	//## list which describes the element to change
    PointOperation(OperationType operationType,
					ITKPoint3D point, int index);

    //##ModelId=3F0189F003B2
    virtual ~PointOperation();

    //##ModelId=3F0189F003B4
	ITKPoint3D GetPoint();

    //##ModelId=3F0189F003B5
	int GetIndex();

  private:
    //##ModelId=3F0189F00393
	ITKPoint3D m_Point;
    //##ModelId=3F0189F003A1
	int m_Index;
};
}//namespace mitk
#endif /* MITKPOINTOPERATION*/