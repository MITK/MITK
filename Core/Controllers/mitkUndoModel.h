#ifndef UNDOMODEL_H_HEADER_INCLUDED_C16ED098
#define UNDOMODEL_H_HEADER_INCLUDED_C16ED098

#include "mitkCommon.h"
#include "OperationEvent.h"

namespace mitk {
//##ModelId=3E5F564C03D4
class UndoModel
{
  public:
    //##ModelId=3E5F5C6C00DF
    virtual bool SetOperationEvent(OperationEvent* operationEvent);

    //##ModelId=3E5F5C6C00F3
    virtual bool Undo();

    //##ModelId=3E5F5C6C00FE
    virtual bool Redo();

};
}// namespace mitk



#endif /* UNDOMODEL_H_HEADER_INCLUDED_C16ED098 */
