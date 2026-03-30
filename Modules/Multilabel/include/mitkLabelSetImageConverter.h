/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLabelSetImageConverter_h
#define mitkLabelSetImageConverter_h

#include <mitkLabelSetImage.h>

namespace mitk
{
  /**
   * \brief Convert mitk::MultiLabelSegmentation to mitk::Image (itk::VectorImage)
   */
  MITKMULTILABEL_EXPORT Image::Pointer ConvertLabelSetImageToImage(MultiLabelSegmentation::ConstPointer labelSetImage);

  /**
   * \brief Convert mitk::Image to mitk::MultiLabelSegmentation, templating and differentation between itk::Image and
   * itk::VectorImage is internal
   */
  MITKMULTILABEL_EXPORT MultiLabelSegmentation::Pointer ConvertImageToLabelSetImage(Image::Pointer image);
  MITKMULTILABEL_EXPORT MultiLabelSegmentation::Pointer ConvertImageVectorToLabelSetImage(const std::vector<mitk::Image::Pointer>& images, const TimeGeometry* timeGeometry);

  MITKMULTILABEL_EXPORT std::vector<mitk::Image::Pointer> SplitVectorImage(const Image* vecImage);

  /** Function takes a vector of labels and transfers all labels as clones with adapted label values to the result vector.
  The values will be adapted according to the provided mapping (key is the old value, value the new).
  \remark: Only labels will be transferred, nothing else. So things like message observers or m_ReservedLabelValuesFunctor must be copied explicitly.*/
  MITKMULTILABEL_EXPORT MultiLabelSegmentation::LabelVectorType GenerateLabelSetWithMappedValues(const MultiLabelSegmentation::ConstLabelVectorType&, LabelValueMappingVector labelMapping);

  MITKMULTILABEL_EXPORT Image::Pointer ConvertImageToGroupImage(const Image* inputImage, mitk::MultiLabelSegmentation::LabelValueVectorType& foundLabels);

  MITKMULTILABEL_EXPORT bool CheckForLabelValueConflictsAndResolve(const mitk::MultiLabelSegmentation::LabelValueVectorType& newValues, mitk::MultiLabelSegmentation::LabelValueVectorType& usedLabelValues, mitk::MultiLabelSegmentation::LabelValueVectorType& correctedLabelValues);

  /** Function creates a binary mask representing only the specified label of the multi label segmentation.
  * \param[in] segmentation Pointer to the segmentation that is the source for the mask.
  * \param[in] labelValue The label that should be extracted.
  * \param[in] createBinaryMap If true, label pixels are set to 1; if false, they keep their label value.
  * \pre segmentation must point to a valid instance.
  * \pre labelValue must exist in segmentation.
  */
  MITKMULTILABEL_EXPORT Image::Pointer CreateLabelMask(const MultiLabelSegmentation* segmentation, MultiLabelSegmentation::LabelValueType labelValue, bool createBinaryMap = true);

  /** Function creates the group image of a segmentation that only contains the selected labels.
  * \param[in] segmentation Pointer to the segmentation that is the source for the map.
  * \param[in] groupID The group that should be used.
  * \param[in] selectedLabels White list of label values to include. Invalid values are ignored.
  * \return Group image clone that only contains the selected values.
  * \pre segmentation must point to a valid instance.
  * \pre groupID must exist in segmentation.
  */
  MITKMULTILABEL_EXPORT Image::Pointer CreateFilteredGroupImage(const MultiLabelSegmentation* segmentation, MultiLabelSegmentation::GroupIndexType groupID, const MultiLabelSegmentation::LabelValueVectorType& selectedLabels);

  using IDToLabelClassNameMapType = std::map<MultiLabelSegmentation::LabelValueType, std::string>;
  /** \brief Create a map of all label classes in a specified group.
  *
  * \param[in] segmentation Pointer to the segmentation that is the source for the map.
  * \param[in] groupID The group that should be used.
  * \param[in] selectedLabels White list of label values to include. Invalid values are ignored.
  * \return A pair: first is the map image, second is the lookup table mapping pixel values to class names.
  * \pre segmentation must point to a valid instance.
  * \pre groupID must exist in segmentation.
  */
  MITKMULTILABEL_EXPORT std::pair<Image::Pointer, IDToLabelClassNameMapType> CreateLabelClassMap(const MultiLabelSegmentation* segmentation, MultiLabelSegmentation::GroupIndexType groupID, const MultiLabelSegmentation::LabelValueVectorType& selectedLabels);

  /** \brief Create a map of all label classes in a specified group.
  *
  * \overload
  * This version always uses all labels of the group.
  *
  * \param[in] segmentation Pointer to the segmentation that is the source for the map.
  * \param[in] groupID The group that should be used.
  * \return A pair: first is the map image, second is the lookup table mapping pixel values to class names.
  * \pre segmentation must point to a valid instance.
  * \pre groupID must exist in segmentation.
  */
  MITKMULTILABEL_EXPORT std::pair<Image::Pointer, IDToLabelClassNameMapType> CreateLabelClassMap(const MultiLabelSegmentation* segmentation, MultiLabelSegmentation::GroupIndexType groupID);

}

#endif
