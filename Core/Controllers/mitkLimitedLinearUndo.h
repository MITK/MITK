#ifndef LIMITEDLINEARUNDO_H_HEADER_INCLUDED_C16E9C96
#define LIMITEDLINEARUNDO_H_HEADER_INCLUDED_C16E9C96

#include "mitkCommon.h"
#include "OperationEvent.h"
#include "UndoModel.h"
#include <vector>

namespace mitk {
//##ModelId=3E5F5D3F0075
class LimitedLinearUndo : public UndoModel
{
public:
    //##ModelId=3E5F5D8C00B2
    virtual bool SetOperationEvent(OperationEvent* operationEvent);

    //##ModelId=3E5F5D8C00C6
    //##Documentation
	//## @brief Undoes the last changes
	//##
    //##  reads the top element of the Undo-Stack,  
    //##  executes the operation,
    //##  swaps the OperationEvent-Undo with the Operation
    //##  and sets it to Redo-Stack
	//##
	//## @params fine: if set to true, then all operations with the same ObjectId are undone
	//## if set to false, then all operations with the same GroupEventId are undone (see UndoController.h)
    virtual bool Undo(bool fine);

    //##ModelId=3E5F5D8C00DA
    //##Documentation
	//## @brief Undoes the last changes
	//##
    //## reads the top element of the Redo-Stack, 
    //##  executes the operation,
    //##  swaps the OperationEvent-Operation with the Undo-Operation
    //##  and sets it to Undo-Stack
    virtual bool Redo();

protected:
    //##ModelId=3E5F5DF80360
    std::vector<OperationEvent*> m_UndoList;

    //##ModelId=3E5F5E020332
    std::vector<OperationEvent*> m_RedoList;

};
}//namespace mitk


#endif /* LIMITEDLINEARUNDO_H_HEADER_INCLUDED_C16E9C96 */
