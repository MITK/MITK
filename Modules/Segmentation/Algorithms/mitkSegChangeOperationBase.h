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
    \sa SegChangeOperationBaseApplier

    The information for the operation is specified by properties:

     imageVolume            the volume where the slice was extracted from.
     slice                  the slice to be applied.
     timestep               the timestep in an 4D image.
     currentWorldGeometry   specifies the axis where the slice has to be applied in the volume.

    This Operation can be used to realize undo-redo functionality for e.g. segmentation purposes.
  */
  class MITKSEGMENTATION_EXPORT SegChangeOperationBase : public Operation
  {
  public:
    mitkClassMacro(SegChangeOperationBase, Operation);

    ~SegChangeOperationBase() override = default;

    /** \brief Check if it is a valid operation.*/
    bool IsValid() const override;

    mitk::MultiLabelSegmentation::Pointer GetSegmentation();
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
