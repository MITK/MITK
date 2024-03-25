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

}

#endif
