/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegGroupInsertOperation_h
#define mitkSegGroupInsertOperation_h

#include "mitkCompressedImageContainer.h"
#include <MitkSegmentationExports.h>
#include <mitkSegChangeOperationBase.h>

namespace mitk
{
  class Image;

  /** \brief An Operation for applying an edited slice to the a group of a MultiLabelSegmentation.
    \sa SegChangeOperationApplier
    This Operation can be used to realize undo-redo functionality for e.g. segmentation purposes.
  */
  class MITKSEGMENTATION_EXPORT SegGroupInsertOperation : public SegChangeOperationBase
  {
  public:
    mitkClassMacro(SegGroupInsertOperation, SegChangeOperationBase);

    using ModifyGroupImageMapType = std::map<MultiLabelSegmentation::GroupIndexType, Image::ConstPointer >;
    using ModifyLabelsMapType = std::map<MultiLabelSegmentation::GroupIndexType, MultiLabelSegmentation::ConstLabelVectorType>;
    using GroupIndexSetType = std::set<MultiLabelSegmentation::GroupIndexType>;
    using ModifyGroupNameMapType = std::map<MultiLabelSegmentation::GroupIndexType, std::string >;

    /** \brief */
    SegGroupInsertOperation(MultiLabelSegmentation* segmentation,
      const GroupIndexSetType& groupIDs,
      const ModifyGroupImageMapType& groupImages = {},
      const ModifyLabelsMapType& groupLabels = {});

    ~SegGroupInsertOperation() override = default;

    GroupIndexSetType GetGroupIDs() const;
    GroupIndexSetType GetImageGroupIDs() const;
    GroupIndexSetType GetLabelGroupIDs() const;

    /** \brief Get the modified group image for a certain group and time step that is applied in the operation.*/
    Image::Pointer GetGroupImage(MultiLabelSegmentation::GroupIndexType groupID) const;
    /** \brief Get the labels for a certain group that is applied in the operation.*/
    MultiLabelSegmentation::ConstLabelVectorType GetGroupLabels(MultiLabelSegmentation::GroupIndexType groupID) const;
    std::string GetGroupName(MultiLabelSegmentation::GroupIndexType groupID) const;

    // Explicitly delete copy operations because internally std::unique_ptr are used.
    SegGroupInsertOperation(const SegGroupInsertOperation&) = delete;
    SegGroupInsertOperation& operator=(const SegGroupInsertOperation&) = delete;

    static SegGroupInsertOperation* CreatFromSegmentation(MultiLabelSegmentation* segmentation,
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
