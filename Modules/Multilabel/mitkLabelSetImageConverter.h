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
  @remark: Only labels will be transferred, nothing else. So things like message observers or m_ReservedLabelValuesFunctor must be copied explicitly.*/
  MITKMULTILABEL_EXPORT MultiLabelSegmentation::LabelVectorType GenerateLabelSetWithMappedValues(const MultiLabelSegmentation::ConstLabelVectorType&, LabelValueMappingVector labelMapping);

  MITKMULTILABEL_EXPORT Image::Pointer ConvertImageToGroupImage(const Image* inputImage, mitk::MultiLabelSegmentation::LabelValueVectorType& foundLabels);

  MITKMULTILABEL_EXPORT bool CheckForLabelValueConflictsAndResolve(const mitk::MultiLabelSegmentation::LabelValueVectorType& newValues, mitk::MultiLabelSegmentation::LabelValueVectorType& usedLabelValues, mitk::MultiLabelSegmentation::LabelValueVectorType& correctedLabelValues);

  /** Function creates a binary mask representing only the specified label of the multi label segmentation.
  * @param segmentation Pointer to the segmentation that is the source for the mask.
  * @param labelValue the label that should be extracted.
  * @param createBinaryMap indicates if the label pixels should be indicated by the value 1 (createBinaryMap==true) or by the value of the label
  * (createBinaryMap==false).
  * @pre segmentation must point to a valid instance.
  * @pre labelValue must exist in segmentation.*/
  MITKMULTILABEL_EXPORT Image::Pointer CreateLabelMask(const MultiLabelSegmentation* segmentation, MultiLabelSegmentation::LabelValueType labelValue, bool createBinaryMap = true);

  using IDToLabelClassNameMapType = std::map<MultiLabelSegmentation::LabelValueType, std::string>;
  /** Function creates a map of all label classes in a specified group.
  * @param segmentation Pointer to the segmentation that is the source for the map.
  * @param groupID the group that should be used.
  * @param selectedLabels The selected labels that should be represented in the class map. This is meant as white list, therefore only
  * label values listed in the list are used. Invalid label values (not existing in the group) will be ignored.
  * @return Returns a pair where first is the pointer to the created map image and second is the look up table that indicated
  * the pixel value of each found class in the map.
  * @pre segmentation must point to a valid instance.
  * @pre groupID must exist in segmentation.*/
  MITKMULTILABEL_EXPORT std::pair<Image::Pointer, IDToLabelClassNameMapType> CreateLabelClassMap(const MultiLabelSegmentation* segmentation, MultiLabelSegmentation::GroupIndexType groupID, const MultiLabelSegmentation::LabelValueVectorType& selectedLabels);

  /** Function creates a map of all label classes in a specified group.
  * @overload
  * This version always uses all labels of a group.
  * @param segmentation Pointer to the segmentation that is the source for the map.
  * @param groupID the group that should be used.
  * @return Returns a pair where first is the pointer to the created map image and second is the look up table that indicated
  * the pixel value of each found class in the map.
  * @pre segmentation must point to a valid instance.
  * @pre groupID must exist in segmentation.*/
  MITKMULTILABEL_EXPORT std::pair<Image::Pointer, IDToLabelClassNameMapType> CreateLabelClassMap(const MultiLabelSegmentation* segmentation, MultiLabelSegmentation::GroupIndexType groupID);

}

#endif
