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
   * \brief Convert mitk::LabelSetImage to mitk::Image (itk::VectorImage)
   */
  MITKMULTILABEL_EXPORT Image::Pointer ConvertLabelSetImageToImage(LabelSetImage::ConstPointer labelSetImage);

  /**
   * \brief Convert mitk::Image to mitk::LabelSetImage, templating and differentation between itk::Image and
   * itk::VectorImage is internal
   */
  MITKMULTILABEL_EXPORT LabelSetImage::Pointer ConvertImageToLabelSetImage(Image::Pointer image);
  MITKMULTILABEL_EXPORT LabelSetImage::Pointer ConvertImageVectorToLabelSetImage(const std::vector<mitk::Image::Pointer>& images, const TimeGeometry* timeGeometry);

  MITKMULTILABEL_EXPORT std::vector<mitk::Image::Pointer> SplitVectorImage(const Image* vecImage);

  /** Function takes a vector of labels and transfers all labels as clones with adapted label values to the result vector.
  The values will be adapted according to the provided mapping (key is the old value, value the new).
  @remark: Only labels will be transfered, nothing else. So things like message observers or m_ReservedLabelValuesFunctor must be copied explicitly.*/
  MITKMULTILABEL_EXPORT LabelSetImage::LabelVectorType GenerateLabelSetWithMappedValues(const LabelSetImage::ConstLabelVectorType&, LabelValueMappingVector labelMapping);

  MITKMULTILABEL_EXPORT Image::Pointer ConvertImageToGroupImage(const Image* inputImage, mitk::LabelSetImage::LabelValueVectorType& foundLabels);

  MITKMULTILABEL_EXPORT bool CheckForLabelValueConflictsAndResolve(const mitk::LabelSetImage::LabelValueVectorType& newValues, mitk::LabelSetImage::LabelValueVectorType& usedLabelValues, mitk::LabelSetImage::LabelValueVectorType& correctedLabelValues);

  /** Function creates a binary mask representing only the specified label of the multi label segmentation.
  * @param segmentation Pointer to the segmentation that is the source for the mask.
  * @param labelValue the label that should be extracted.
  * @param createBinaryMap indicates if the label pixels should be indicated by the value 1 (createBinaryMap==true) or by the value of the label
  * (createBinaryMap==false).
  * @pre segmentation must point to a valid instance.
  * @pre labelValue must exist in segmentation.*/
  MITKMULTILABEL_EXPORT Image::Pointer CreateLabelMask(const LabelSetImage* segmentation, LabelSetImage::LabelValueType labelValue, bool createBinaryMap = true);

  /** Function creates a map of all label classes in a specified group.
  * @param segmentation Pointer to the segmentation that is the source for the map.
  * @param groupID the group that should be used.
  * @param selectedLabels The selected labels that should be represented in the class map. This is meant as white list, therefore only
  * label values listed in the list are used. Invalid label values (not existing in the group) will be ignored.
  * @return Returns a pair where first is the pointer to the created map image and second is the look up table that indicated
  * the pixel value of each found class in the map.
  * @pre segmentation must point to a valid instance.
  * @pre groupID must exist in segmentation.*/
  using IDToLabelClassNameMapType = std::map<LabelSetImage::LabelValueType, std::string>;
  MITKMULTILABEL_EXPORT std::pair<Image::Pointer, IDToLabelClassNameMapType> CreateLabelClassMap(const LabelSetImage* segmentation, LabelSetImage::GroupIndexType groupID, const LabelSetImage::LabelValueVectorType& selectedLabels);
  /** Function creates a map of all label classes in a specified group.
  * @overload
  * This version always uses all labels of a group.
  * @param segmentation Pointer to the segmentation that is the source for the map.
  * @param groupID the group that should be used.
  * @return Returns a pair where first is the pointer to the created map image and second is the look up table that indicated
  * the pixel value of each found class in the map.
  * @pre segmentation must point to a valid instance.
  * @pre groupID must exist in segmentation.*/
  MITKMULTILABEL_EXPORT std::pair<Image::Pointer, IDToLabelClassNameMapType> CreateLabelClassMap(const LabelSetImage* segmentation, LabelSetImage::GroupIndexType groupID);

}

#endif
