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
    //##ModelId=3E5F5D8C00B2
    virtual bool SetOperationEvent(OperationEvent* operationEvent);

    //##ModelId=3E5F5D8C00C6
    //##Documentation
    //##  reads the top element of the Undo-Stack,  
    //##  executes the operation,
    //##  swaps the OperationEvent-Undo with the Operation
    //##  and sets it to Redo-Stack
    virtual bool Undo();

    //##ModelId=3E5F5D8C00DA
    //##Documentation
    //## reads the top element of the Redo-Stack, 
    //##  executes the operation,
    //##  swaps the OperationEvent-Operation with the Undo-Operation
    //##  and sets it to Undo-Stack
    virtual bool Redo();

    //##ModelId=3E5F5DF80360
    std::vector<OperationEvent*> m_UndoList;

    //##ModelId=3E5F5E020332
    std::vector<OperationEvent*> m_RedoList;

};
}//namespace mitk


#endif /* LIMITEDLINEARUNDO_H_HEADER_INCLUDED_C16E9C96 */
