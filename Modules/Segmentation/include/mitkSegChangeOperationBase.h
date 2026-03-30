/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegChangeOperationBase_h
#define mitkSegChangeOperationBase_h

#include <mitkLabelSetImage.h>
#include <mitkOperation.h>
#include <mitkWeakPointer.h>

#include <MitkSegmentationExports.h>

namespace mitk
{
  class Image;

  /** \brief Base class for any operation based on a MultiLabelSegmentation instance.

    \sa SegChangeOperationApplier

    This Operation serves as the common base for all undo/redo operations that modify
    a MultiLabelSegmentation. It holds a weak reference to the target segmentation
    and provides validity checking.

    This Operation can be used to realize undo-redo functionality for e.g. segmentation purposes.
  */
  class MITKSEGMENTATION_EXPORT SegChangeOperationBase : public Operation
  {
  public:
    mitkClassMacro(SegChangeOperationBase, Operation);

    ~SegChangeOperationBase() override = default;

    /** \brief Check whether the operation is still valid (i.e., the segmentation still exists). */
    bool IsValid() const override;

    /** \brief Get the target segmentation (mutable). */
    mitk::MultiLabelSegmentation::Pointer GetSegmentation();

    /** \brief Get the target segmentation (const). */
    const mitk::MultiLabelSegmentation::ConstPointer GetSegmentation() const;

  protected:
    SegChangeOperationBase(mitk::MultiLabelSegmentation* segmentation, OperationType operationType);
    SegChangeOperationBase(SegChangeOperationBase&&) = default;
    SegChangeOperationBase& operator=(const SegChangeOperationBase&) = default;
    SegChangeOperationBase& operator=(SegChangeOperationBase&&) = default;

  private:
    WeakPointer<MultiLabelSegmentation> m_Segmentation;
  };
}
#endif
