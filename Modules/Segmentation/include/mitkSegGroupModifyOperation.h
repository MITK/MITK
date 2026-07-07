/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegGroupModifyOperation_h
#define mitkSegGroupModifyOperation_h

#include <mitkCompressedImageContainer.h>
#include <MitkSegmentationExports.h>
#include <mitkSegChangeOperationBase.h>

namespace mitk
{
  class Image;

  /** \brief An Operation for modifying existing groups of a MultiLabelSegmentation.

    \sa SegChangeOperationApplier

    This Operation stores modified group images, labels, and names and can be used
    to realize undo-redo functionality for segmentation group modifications.
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

    /** \brief Construct a group modify operation with the given modified data.
      \param segmentation The target segmentation.
      \param modifiedGroupImages Map of group index to time-step-indexed modified images.
      \param modifiedLabels Map of group index to modified label vectors.
      \param modifiedNames Map of group index to modified group names.
    */
    SegGroupModifyOperation(MultiLabelSegmentation* segmentation,
      const ModifyGroupImageMapType& modifiedGroupImages,
      const ModifyLabelsMapType& modifiedLabels,
      const ModifyGroupNameMapType& modifiedNames);

    ~SegGroupModifyOperation() override = default;

    /** \brief Get the group indices that have modified image data. */
    GroupIndexVectorType GetImageGroupIDs() const;

    /** \brief Get the time steps with modified images for a specific group. */
    TimeStepVectorType GetImageTimeSteps(MultiLabelSegmentation::GroupIndexType groupID) const;

    /** \brief Get the group indices that have modified label data. */
    GroupIndexVectorType GetLabelGroupIDs() const;

    /** \brief Get the group indices that have modified names. */
    GroupIndexVectorType GetNameGroupIDs() const;

    /** \brief Get the modified group image for a certain group and time step.
      \param groupID The group index.
      \param timeStep The time step.
      \return The stored modified image, decompressed from the internal container.
    */
    Image::Pointer GetModifiedGroupImage(MultiLabelSegmentation::GroupIndexType groupID, TimeStepType timeStep) const;

    /** \brief Get the modified labels for a certain group.
      \param groupID The group index.
    */
    MultiLabelSegmentation::ConstLabelVectorType GetModifiedLabels(MultiLabelSegmentation::GroupIndexType groupID) const;

    /** \brief Get the modified name for a certain group. */
    std::string GetModifiedName(MultiLabelSegmentation::GroupIndexType groupID) const;

    // Explicitly delete copy operations because internally std::unique_ptr are used.
    SegGroupModifyOperation(const SegGroupModifyOperation&) = delete;
    SegGroupModifyOperation& operator=(const SegGroupModifyOperation&) = delete;

    /** \brief Factory method that creates a modify operation from the current state of a segmentation.
      \param segmentation The segmentation to capture state from.
      \param relevantGroupIDs The group indices to capture.
      \param coverAllTimeSteps If true, all time steps are captured; otherwise only the specified one.
      \param timeStep The time step to capture (only used if coverAllTimeSteps is false).
      \param noLabels If true, label information is not captured.
      \param noGroupImages If true, group image data is not captured.
      \param noNames If true, group names are not captured.
    */
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
