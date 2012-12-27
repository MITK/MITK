/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef MODEOPERATION_H_HEADER_INCLUDED
#define MODEOPERATION_H_HEADER_INCLUDED

#include <MitkExports.h>
#include "mitkOperation.h"
#include <mitkInteractor.h>

namespace mitk {
//  class Operation;

//##Documentation
//## @brief class that holds the information for a change of the modus of an interactor object
//##
//## @ingroup Undo
class MITK_CORE_EXPORT ModeOperation : public Operation
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
