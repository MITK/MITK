#ifndef OPERATION_H_HEADER_INCLUDED_C16E7D9E
#define OPERATION_H_HEADER_INCLUDED_C16E7D9E

#include "mitkCommon.h"
//##ModelId=3E9B07B10043

namespace mitk {
//##ModelId=3EA54FFA00B5
typedef int OperationType ;
//static const int OpADD = 100;//add at the end
//static const int OpINSERT = 200;//insert at position
//static const int OpMOVE = 300;//move
//static const int OpREMOVE = 400;//remove at position
//static const int OpDELETE = 500;//delete from the end
//static const int OpSTATECHANGE = 600;//change a state
//static const int OpSELECTPOINT = 700;
//static const int OpDESELECTPOINT = 800;
//static const int OpNOTHING = 999;



//##ModelId=3E5F682C0289
//##Documentation
//## @brief Base class of all Operation-classes
//## @ingroup Undo
//## 
class Operation
{
public:
  //##ModelId=3E7830E70054
  //##Documentation
	//## Constructor
	//## @params execId: which Operation shall be executed in Execute()
  //## 
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
