#ifndef MITKLINEOPERATION_H_INCLUDED
#define MITKLINEOPERATION_H_INCLUDED

#include "mitkCommon.h"
#include "mitkOperation.h"
#include "mitkVector.h"


namespace mitk {

//##Documentation
//## @brief Operation, that holds everything necessary for an operation on a line.
//##
//## @ingroup Undo
//##
//## Stores everything for de-/ selecting, inserting , moving and removing a line.
class LineOperation : public mitk::Operation
{
  public:
	//##Documentation
	//##@brief constructor.
	//##
	//## @params
	//## operationType is the type of that operation (see mitkOperation.h; e.g. move or add; Information for StateMachine::ExecuteOperation());
	//## point is the information of the point to add or is the information to change a point into; index is e.g. the position in a
	//## list which describes the element to change
    LineOperation(OperationType operationType,
					int cellId, int pIdA, int pIdB);

    virtual ~LineOperation();

	int GetCellId();
	int GetPIdA();
  int GetPIdB();

  protected:
  int m_CellId;
	int m_PIdA;
  int m_PIdB;
};
}//namespace mitk
#endif /* MITKLINEOPERATION_H_INCLUDED*/

