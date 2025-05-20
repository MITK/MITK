/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegLabelPropModifyOperation_h
#define mitkSegLabelPropModifyOperation_h

#include "mitkCompressedImageContainer.h"
#include <MitkSegmentationExports.h>
#include <mitkSegChangeOperationBase.h>

namespace mitk
{
  class Image;

  /** \brief An Operation for applying a modification to the properties of a label (E.g. name, color) of a MultiLabelSegmentation.
    \sa SegOperationApplier
    This Operation can be used to realize undo-redo functionality for changing of label properties.
    If you want to add/remove labels from a segmentation (w/ or w/o the pixel data), use the
    SegGroupModifyOperation.
  */
  class MITKSEGMENTATION_EXPORT SegLabelPropModifyOperation : public SegChangeOperationBase
  {
  public:
    mitkClassMacro(SegLabelPropModifyOperation, SegChangeOperationBase);

    using ModifyLabelsVectorType = MultiLabelSegmentation::ConstLabelVectorType;
    
    /** \brief */
    SegLabelPropModifyOperation(MultiLabelSegmentation* segmentation,
      const ModifyLabelsVectorType& modifiedLabel);

    ~SegLabelPropModifyOperation() override = default;

    /** \brief Get the modified labels map.*/
    const ModifyLabelsVectorType& GetModifiedLabels() const;

    // Explicitly delete copy operations because internally std::unique_ptr are used.
    SegLabelPropModifyOperation(const SegLabelPropModifyOperation&) = delete;
    SegLabelPropModifyOperation& operator=(const SegLabelPropModifyOperation&) = delete;

    static SegLabelPropModifyOperation* CreatFromSegmentation(MultiLabelSegmentation* segmentation,
      const MultiLabelSegmentation::LabelValueVectorType& relevantLabels);

  protected:
    ModifyLabelsVectorType m_ModifiedLabels;
  };

}
#endif
