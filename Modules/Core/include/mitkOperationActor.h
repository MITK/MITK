/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef OPERATIONACTOR_H_HEADER_INCLUDED_C16E28BD
#define OPERATIONACTOR_H_HEADER_INCLUDED_C16E28BD

#include <MitkCoreExports.h>
#include <mitkCommon.h>

/** Macro for checking the type of an operation */
#define mitkCheckOperationTypeMacro(OperationType, operation, newOperationName)                                        \
                                                                                                                       \
  OperationType *newOperationName = dynamic_cast<OperationType *>(operation);                                          \
                                                                                                                       \
  if (newOperationName == nullptr)                                                                                        \
                                                                                                                       \
  {                                                                                                                    \
    itkWarningMacro("Recieved wrong type of operation!");                                                              \
    return;                                                                                                            \
  }

namespace mitk
{
  class Operation;
  class OperationEvent;

  /**
   * \brief abstract class, that can be used  by Undo to undo an operation.
   *
   * \ingroup Undo
   */
  class MITKCORE_EXPORT OperationActor
  {
  public:
    itkTypeMacroNoParent(OperationActor)

      virtual ~OperationActor()
    {
    }
    virtual void ExecuteOperation(Operation *operation) = 0;
  };
}

#endif /* OPERATIONACTOR_H_HEADER_INCLUDED_C16E28BD */
