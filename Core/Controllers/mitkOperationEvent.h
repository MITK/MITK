#ifndef OPERATIONEVENT_H_HEADER_INCLUDED_C16E83FC
#define OPERATIONEVENT_H_HEADER_INCLUDED_C16E83FC

#include "mitkCommon.h"
#include "Operation.h"
#include "OperationActor.h"

namespace mitk {
//##ModelId=3E5F60F301A4
class OperationEvent
{
  public:
    //##ModelId=3E5F610D00BB
    Operation GetOperation();

    //##ModelId=3E5F6123006D
    Operation GetUndoOperation();

    //##ModelId=3E957AE700E6
    OperationEvent(OperationActor destination, Operation* operation, Operation* undoOperation, int objectEventId, int groupEventId);


  protected:
    //##ModelId=3E957C1102E3
    void swapOperations();



  private:
    //##ModelId=3E5F61DB00D6
    OperationActor m_Destination;

    //##ModelId=3E5F6B1E0107
    Operation* m_Operation;

    //##ModelId=3E5F6B2E0060
    Operation* m_UndoOperation;

    //##ModelId=3E7F488E022B
    int m_ObjectEventId;

    //##ModelId=3E7F48C60335
    int m_GroupEventId;

};
} //namespace mitk


#endif /* OPERATIONEVENT_H_HEADER_INCLUDED_C16E83FC */
