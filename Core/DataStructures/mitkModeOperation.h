#ifndef MODEOPERATION_H_HEADER_INCLUDED
#define MODEOPERATION_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkOperation.h"
#include <mitkInteractor.h>

namespace mitk {
//  class Operation;

//##Documentation
//## @brief class that holds the information for a change of the modus of an interactor object
//## @ingroup Undo
class ModeOperation : public Operation
{
public:
  typedef Interactor::ModeType ModeType;

  mitkClassMacro(ModeOperation, Operation);
  //##Documentation
	//## Constructor
  ModeOperation(OperationType operationType, ModeType mode);

  virtual ~ModeOperation();

  ModeType GetMode();

protected:
  
  ModeType m_Mode;
};
}//namespace mitk
#endif /* MODEOPERATION_H_HEADER_INCLUDED */
