/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegGroupModifyOperation_h
#define mitkSegGroupModifyOperation_h

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
  class MITKSEGMENTATION_EXPORT SegGroupModifyOperation : public SegChangeOperationBase
  {
  public:
    mitkClassMacro(SegGroupModifyOperation, SegChangeOperationBase);

    using ModifyTSImageMapType = std::map<TimeStepType, Image::ConstPointer>;
    using ModifyGroupImageMapType = std::map<MultiLabelSegmentation::GroupIndexType, ModifyTSImageMapType >;
    using ModifyLabelsMapType = std::map<MultiLabelSegmentation::GroupIndexType, MultiLabelSegmentation::ConstLabelVectorType>;
    using GroupIndexVectorType = std::vector<MultiLabelSegmentation::GroupIndexType>;
    using TimeStepVectorType = std::vector<TimeStepType>;
    using ModifyGroupNameMapType = std::map<MultiLabelSegmentation::GroupIndexType, std::string >;

    /** \brief */
    SegGroupModifyOperation(MultiLabelSegmentation* segmentation,
      const ModifyGroupImageMapType& modifiedGroupImages,
      const ModifyLabelsMapType& modifiedLabels,
      const ModifyGroupNameMapType& modifiedNames);

    ~SegGroupModifyOperation() override = default;

    GroupIndexVectorType GetImageGroupIDs() const;
    TimeStepVectorType GetImageTimeSteps(MultiLabelSegmentation::GroupIndexType groupID) const;

    GroupIndexVectorType GetLabelGroupIDs() const;
    GroupIndexVectorType GetNameGroupIDs() const;

    /** \brief Get the modified group image for a certain group and time step that is applied in the operation.*/
    Image::Pointer GetModifiedGroupImage(MultiLabelSegmentation::GroupIndexType groupID, TimeStepType timeStep) const;
    /** \brief Get the modified group image for a certain group and time step that is applied in the operation.*/
    MultiLabelSegmentation::ConstLabelVectorType GetModifiedLabels(MultiLabelSegmentation::GroupIndexType groupID) const;
    std::string GetModifiedName(MultiLabelSegmentation::GroupIndexType groupID) const;

    // Explicitly delete copy operations because internally std::unique_ptr are used.
    SegGroupModifyOperation(const SegGroupModifyOperation&) = delete;
    SegGroupModifyOperation& operator=(const SegGroupModifyOperation&) = delete;

    static SegGroupModifyOperation* CreatFromSegmentation(MultiLabelSegmentation* segmentation,
      const std::set<MultiLabelSegmentation::GroupIndexType>& relevantGroupIDs, bool coverAllTimeSteps, TimeStepType timeStep = 0,
      bool noLabels = false, bool noGroupImages = false, bool noNames = false);

  protected:
    using ModifyCompressedImageMapType = std::map<MultiLabelSegmentation::GroupIndexType, std::map<TimeStepType, std::unique_ptr<CompressedImageContainer>>>;
    ModifyCompressedImageMapType m_ModifiedImages;
    ModifyLabelsMapType m_ModifiedLabels;
    ModifyGroupNameMapType m_ModifiedNames;
  };

}
#endif
