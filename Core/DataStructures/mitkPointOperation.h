#ifndef MITKPOINTOPERATION_H
#define MITKPOINTOPERATION_H

#include "mitkCommon.h"
#include "mitkOperation.h"
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
	//## operationType is the type of that operation (see mitkOperation.h; e.g. move or add; Information for StateMachine::ExecuteOperation());
	//## point is the information of the point to add or is the information to change a point into; index is e.g. the position in a
	//## list which describes the element to change
  PointOperation(OperationType operationType,	ITKPoint3D point);
  PointOperation(OperationType operationType,	ITKPoint3D point, int index);
  PointOperation(OperationType operationType,	ITKPoint3D point, int index, PointSpecificationType type);

  //##ModelId=3F0189F003B2
  virtual ~PointOperation();

  //##ModelId=3F0189F003B4
	ITKPoint3D GetPoint();

  //##ModelId=3F0189F003B5
	int GetIndex();

  PointSpecificationType GetPointType();

  private:
  //##ModelId=3F0189F00393
	ITKPoint3D m_Point;

  //##ModelId=3F0189F003A1
  //##Documentation
  //##@brief to declare an index where to store the point in data
	int m_Index;
  
  //##Documentation
  //##@brief to describe the type of the point. See enum PointSpecification for different types
  PointSpecificationType m_Type;
};
}//namespace mitk
#endif /* MITKPOINTOPERATION_H*/

