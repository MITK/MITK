#ifndef OPERATIONACTOR_H_HEADER_INCLUDED_C16E28BD
#define OPERATIONACTOR_H_HEADER_INCLUDED_C16E28BD

#include "mitkCommon.h"
//#include "OperationEvent.h"

namespace mitk 
{
class Operation;
class OperationEvent;
//##ModelId=3E5F62430388
class OperationActor
{
public:
    //##ModelId=3E5F62530359
    virtual void ExecuteOperation(Operation* operation) = 0;

};
}


#endif /* OPERATIONACTOR_H_HEADER_INCLUDED_C16E28BD */
