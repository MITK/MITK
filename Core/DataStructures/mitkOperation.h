#ifndef OPERATION_H_HEADER_INCLUDED_C16E7D9E
#define OPERATION_H_HEADER_INCLUDED_C16E7D9E

#include "mitkCommon.h"
//##ModelId=3E9B07B10043

namespace mitk {
//##ModelId=3EA54FFA00B5
typedef int OperationType ;
static const int ROI = 100;
static const int POINT = 101;
static const int FOCUS = 200;
static const int STATETRANSITION = 300;

//##ModelId=3E5F682C0289
//##Documentation
//## @brief Base class of all Operation-classes
//## @ingroup Undo
//## 
//## 
class Operation
{
  public:
    //##ModelId=3E7830E70054
    //##Documentation
	//## Constructor
	//## @params execId: which Operation shall be executed in Execute()
    //## 
    Operation(OperationType operationType, int execId);

    //##ModelId=3E7831B600CA
    OperationType GetOperationType();

    //##ModelId=3EDDD7640079
	int GetExecutionId();

    //##ModelId=3E7F4D7D01AE
	//##Documentation
	//## executes an implemented Operation 
	//## addressed by the Id
    virtual void Execute() = 0;
	
  protected:
    //##ModelId=3E5F6A9001E9
    OperationType m_OperationType;

    //##ModelId=3EDDD764003A
	int m_ExecutionId;
};
}//namespace mitk
#endif /* OPERATION_H_HEADER_INCLUDED_C16E7D9E */
