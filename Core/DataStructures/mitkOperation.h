#ifndef OPERATION_H_HEADER_INCLUDED_C16E7D9E
#define OPERATION_H_HEADER_INCLUDED_C16E7D9E

#include "mitkCommon.h"
//##ModelId=3E9B07B10043

namespace mitk {
//##ModelId=3EA54FFA00B5
typedef int OperationType ;

//##ModelId=3E5F682C0289
//##Documentation
//## @brief Base class of all Operation-classes
//## @ingroup Undo
class Operation
{
public:
  //##ModelId=3E7830E70054
  //##Documentation
	//## Constructor
  Operation(OperationType operationType);

  //##ModelId=3F01770A0007
  virtual ~Operation();

  //##ModelId=3E7831B600CA
  OperationType GetOperationType();

  protected:
  //##ModelId=3E5F6A9001E9
  OperationType m_OperationType;
};
}//namespace mitk
#endif /* OPERATION_H_HEADER_INCLUDED_C16E7D9E */
