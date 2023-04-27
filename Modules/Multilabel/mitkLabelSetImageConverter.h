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

  /** Function takes a label set and transfers all labels indicated in the label mapping (first element of pair) into a result label set. In the result label set
  the cloned labels will have the label value indicated by the mapping (second element of pair).
  @remark: Only labels will be transfered, nothing else. So things like message observers or m_ReservedLabelValuesFunctor must be copied explicitly.*/
  MITKMULTILABEL_EXPORT LabelSet::Pointer GenerateLabelSetWithMappedValues(const LabelSet* sourceLabelset, std::vector<std::pair<Label::PixelType, Label::PixelType> > labelMapping);
}

#endif
