#ifndef UNDOCONTROLLER_H_HEADER_INCLUDED_C16EFF79
#define UNDOCONTROLLER_H_HEADER_INCLUDED_C16EFF79

#include "mitkCommon.h"
#include "UndoModel.h"
#include "OperationEvent.h"

namespace mitk {
//##ModelId=3E93EE1D001F
	enum UndoType {
	LimitedLinearUndo,
	TreeUndo};

//##ModelId=3E5F53AA026C
class UndoController
{
  public:
    //##ModelId=3E5F543402C5
    bool SetOperationEvent(OperationEvent* operationEvent);

    //##ModelId=3E5F55C80207
    bool Undo();

    //##ModelId=3E5F55E6003E
    bool Redo();

    //##ModelId=3E5F56EB017A
    bool SetUndoType(UndoType undoType);

  private:
    //##ModelId=3E5F5DC701C5
    UndoModel m_UndoModel;

};
}//namespace mitk



#endif /* UNDOCONTROLLER_H_HEADER_INCLUDED_C16EFF79 */
