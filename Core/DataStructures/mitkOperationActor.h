#ifndef OPERATIONACTOR_H_HEADER_INCLUDED_C16E28BD
#define OPERATIONACTOR_H_HEADER_INCLUDED_C16E28BD

#include "mitkCommon.h"

namespace mitk 
{

class Operation;
class OperationEvent;

/** Macro for checking the type of an operation*/
#define mitkCheckOperationTypeMacro(OperationType, operation, newOperationName) \
  mitk::OperationType *newOperationName= dynamic_cast<mitk::OperationType *>(operation);\
	if (newOperationName == NULL)\
	{\
    itkWarningMacro("Recieved wrong type of operation!");\
		return;\
	}\

//##ModelId=3E5F62430388
//##Documentation
//## @brief abstract class, that can be used  by Undo to undo an operation.
//## @ingroup Undo
//## 
class OperationActor
{
public:
    //##ModelId=3E5F62530359
    virtual void ExecuteOperation(Operation* operation) = 0;
};
}


#endif /* OPERATIONACTOR_H_HEADER_INCLUDED_C16E28BD */
