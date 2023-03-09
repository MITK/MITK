/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDiffSliceOperationApplier_h
#define mitkDiffSliceOperationApplier_h

#include "mitkCommon.h"
#include <MitkSegmentationExports.h>
#include <mitkOperationActor.h>

namespace mitk
{
  /** \brief Executes a DiffSliceOperation.
    \sa DiffSliceOperation
  */
  class MITKSEGMENTATION_EXPORT DiffSliceOperationApplier : public OperationActor
  {
  public:
    mitkClassMacroNoParent(DiffSliceOperationApplier)

      /** \brief Returns an instance of the class */
      static DiffSliceOperationApplier *GetInstance();

    /** \brief Executes a DiffSliceOperation.
      \sa DiffSliceOperation
      Note:
        Only DiffSliceOperation is supported.
    */
    void ExecuteOperation(Operation *op) override;

  protected:
    DiffSliceOperationApplier();

    ~DiffSliceOperationApplier() override;
  };
}
#endif
