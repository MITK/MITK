/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegGroupModifyOperationApplier_h
#define mitkSegGroupModifyOperationApplier_h

#include "mitkCommon.h"
#include <mitkOperationActor.h>
#include <MitkSegmentationExports.h>

namespace mitk
{
  /** \brief Executes a SegGroupModifyOperation.
    \sa SegGroupModifyOperation
  */
  class MITKSEGMENTATION_EXPORT SegGroupModifyOperationApplier : public OperationActor
  {
  public:
    mitkClassMacroNoParent(SegGroupModifyOperationApplier)

      /** \brief Returns an instance of the class */
      static SegGroupModifyOperationApplier *GetInstance();

    /** \brief Executes a SegGroupModifyOperation.
      \sa SegGroupModifyOperation
      Note:
        Only SegGroupModifyOperation is supported.
    */
    void ExecuteOperation(Operation *op) override;

  protected:
    SegGroupModifyOperationApplier();

    ~SegGroupModifyOperationApplier() override;
  };
}
#endif
