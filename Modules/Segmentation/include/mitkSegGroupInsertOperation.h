/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegGroupInsertOperation_h
#define mitkSegGroupInsertOperation_h

#include <mitkCompressedImageContainer.h>
#include <MitkSegmentationExports.h>
#include <mitkSegChangeOperationBase.h>

namespace mitk
{
  class Image;

  /** \brief An Operation for inserting groups into a MultiLabelSegmentation.

    \sa SegChangeOperationApplier

    This Operation can be used to realize undo-redo functionality for group
    insertion in segmentation workflows. It stores group images, labels, and names
    to be inserted at specific group indices.
  */
  class MITKSEGMENTATION_EXPORT SegGroupInsertOperation : public SegChangeOperationBase
  {
  public:
    mitkClassMacro(SegGroupInsertOperation, SegChangeOperationBase);

    using ModifyGroupImageMapType = std::map<MultiLabelSegmentation::GroupIndexType, Image::ConstPointer >;
    using ModifyLabelsMapType = std::map<MultiLabelSegmentation::GroupIndexType, MultiLabelSegmentation::ConstLabelVectorType>;
    using GroupIndexSetType = std::set<MultiLabelSegmentation::GroupIndexType>;
    using ModifyGroupNameMapType = std::map<MultiLabelSegmentation::GroupIndexType, std::string >;

    /** \brief Construct an insert operation with the given group data.
      \param segmentation The target segmentation.
      \param groupIDs The set of group indices to insert.
      \param groupImages Optional map of group images to insert.
      \param groupLabels Optional map of labels to insert for each group.
    */
    SegGroupInsertOperation(MultiLabelSegmentation* segmentation,
      const GroupIndexSetType& groupIDs,
      const ModifyGroupImageMapType& groupImages = {},
      const ModifyLabelsMapType& groupLabels = {});

    ~SegGroupInsertOperation() override = default;

    /** \brief Get all group indices managed by this operation. */
    GroupIndexSetType GetGroupIDs() const;

    /** \brief Get the group indices that have associated image data. */
    GroupIndexSetType GetImageGroupIDs() const;

    /** \brief Get the group indices that have associated label data. */
    GroupIndexSetType GetLabelGroupIDs() const;

    /** \brief Get the group image for a specific group index.
      \param groupID The group index.
      \return The stored group image, decompressed from the internal container.
    */
    Image::Pointer GetGroupImage(MultiLabelSegmentation::GroupIndexType groupID) const;

    /** \brief Get the labels for a specific group index. */
    MultiLabelSegmentation::ConstLabelVectorType GetGroupLabels(MultiLabelSegmentation::GroupIndexType groupID) const;

    /** \brief Get the group name for a specific group index. */
    std::string GetGroupName(MultiLabelSegmentation::GroupIndexType groupID) const;

    // Explicitly delete copy operations because internally std::unique_ptr are used.
    SegGroupInsertOperation(const SegGroupInsertOperation&) = delete;
    SegGroupInsertOperation& operator=(const SegGroupInsertOperation&) = delete;

    /** \brief Factory method that creates an insert operation from the current state of a segmentation.
      \param segmentation The segmentation to capture state from.
      \param relevantGroupIDs The group indices to capture.
      \param noLabels If true, label information is not captured.
      \param noGroupImages If true, group image data is not captured.
    */
    static SegGroupInsertOperation* CreateFromSegmentation(MultiLabelSegmentation* segmentation,
      const GroupIndexSetType& relevantGroupIDs,
      bool noLabels = false, bool noGroupImages = false);

  protected:
    using ModifyCompressedImageMapType = std::map<MultiLabelSegmentation::GroupIndexType, std::unique_ptr<CompressedImageContainer>>;
    GroupIndexSetType m_GroupIDs;
    ModifyCompressedImageMapType m_Images;
    ModifyLabelsMapType m_Labels;
    ModifyGroupNameMapType m_Names;
  };

}
#endif
