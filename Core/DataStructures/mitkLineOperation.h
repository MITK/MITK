#ifndef MITKLINEOPERATION_H_INCLUDED
#define MITKLINEOPERATION_H_INCLUDED

#include "mitkCommon.h"
#include "mitkCellOperation.h"


namespace mitk {

//##Documentation
//## @brief Operation, that holds everything necessary for an operation on a line.
//##
//## @ingroup Undo
//##
//## Stores everything for de-/ selecting, inserting , moving and removing a line.
class LineOperation : public mitk::CellOperation
{
  public:
    mitkClassMacro(LineOperation, CellOperation);
	//##Documentation
	//##@brief constructor.
	//##
	//## @params
	//## operationType is the type of that operation (see mitkOperation.h; e.g. move or add; Information for StateMachine::ExecuteOperation());
	//## other Id's are to transmitt parameters for the operation: cellId: Id of the cell; a vector for movement; pIdA, pIdB: Id's of two points; id: the Id of a line in a cell
    LineOperation(OperationType operationType, int cellId, Vector3D vector, int pIdA = -1, int pIdB = -1, int id = -1);
    LineOperation(OperationType operationType, int cellId = -1, int pIdA = -1, int pIdB = -1, int id = -1);
  
    virtual ~LineOperation(){};

	//int GetCellId();
	int GetPIdA();
  int GetPIdB();
  int GetId();

  protected:
//  int m_CellId;
  int m_PIdA;
  int m_PIdB;
  int m_Id;
};
}//namespace mitk
#endif /* MITKLINEOPERATION_H_INCLUDED*/

