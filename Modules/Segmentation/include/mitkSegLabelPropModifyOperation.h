/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegLabelPropModifyOperation_h
#define mitkSegLabelPropModifyOperation_h

#include <mitkCompressedImageContainer.h>
#include <MitkSegmentationExports.h>
#include <mitkSegChangeOperationBase.h>

namespace mitk
{
  class Image;

  /** \brief An Operation for modifying the properties (e.g. name, color) of labels in a MultiLabelSegmentation.

    \sa SegChangeOperationApplier

    This Operation can be used to realize undo-redo functionality for changing label properties.
    If you want to add/remove labels from a segmentation (with or without pixel data), use
    SegGroupModifyOperation instead.
  */
  class MITKSEGMENTATION_EXPORT SegLabelPropModifyOperation : public SegChangeOperationBase
  {
  public:
    mitkClassMacro(SegLabelPropModifyOperation, SegChangeOperationBase);

    using ModifyLabelsVectorType = MultiLabelSegmentation::ConstLabelVectorType;

    /** \brief Construct a label property modify operation with the given label data.
      \param segmentation The target segmentation.
      \param modifiedLabel The vector of modified label objects containing new property values.
    */
    SegLabelPropModifyOperation(MultiLabelSegmentation* segmentation,
      const ModifyLabelsVectorType& modifiedLabel);

    ~SegLabelPropModifyOperation() override = default;

    /** \brief Get the vector of modified labels stored in this operation. */
    const ModifyLabelsVectorType& GetModifiedLabels() const;

    // Explicitly delete copy operations because internally std::unique_ptr are used.
    SegLabelPropModifyOperation(const SegLabelPropModifyOperation&) = delete;
    SegLabelPropModifyOperation& operator=(const SegLabelPropModifyOperation&) = delete;

    /** \brief Factory method that creates a label property modify operation from the current segmentation state.
      \param segmentation The segmentation to capture label properties from.
      \param relevantLabels The label values whose properties should be captured.
    */
    static SegLabelPropModifyOperation* CreatFromSegmentation(MultiLabelSegmentation* segmentation,
      const MultiLabelSegmentation::LabelValueVectorType& relevantLabels);

  protected:
    ModifyLabelsVectorType m_ModifiedLabels;
  };

}
#endif
