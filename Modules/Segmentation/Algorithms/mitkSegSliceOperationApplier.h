/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegSliceOperationApplier_h
#define mitkSegSliceOperationApplier_h

#include "mitkCommon.h"
#include <mitkOperationActor.h>
#include <MitkSegmentationExports.h>

namespace mitk
{
  /** \brief Executes a SegSliceOperation.
    \sa SegSliceOperation
  */
  class MITKSEGMENTATION_EXPORT SegSliceOperationApplier : public OperationActor
  {
  public:
    mitkClassMacroNoParent(SegSliceOperationApplier)

      /** \brief Returns an instance of the class */
      static SegSliceOperationApplier *GetInstance();

    /** \brief Executes a SegSliceOperation.
      \sa SegSliceOperation
      Note:
        Only SegSliceOperation is supported.
    */
    void ExecuteOperation(Operation *op) override;

  protected:
    SegSliceOperationApplier();

    ~SegSliceOperationApplier() override;
  };
}
#endif
