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


#ifndef OPERATIONACTOR_H_HEADER_INCLUDED_C16E28BD
#define OPERATIONACTOR_H_HEADER_INCLUDED_C16E28BD

#include <MitkExports.h>

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

  //##Documentation
  //## @brief abstract class, that can be used  by Undo to undo an operation.
  //##
  //## @ingroup Undo
  class MITK_CORE_EXPORT OperationActor
  {
  public:
    virtual ~OperationActor() {};
    virtual void ExecuteOperation(Operation* operation) = 0;
  };
}


#endif /* OPERATIONACTOR_H_HEADER_INCLUDED_C16E28BD */
