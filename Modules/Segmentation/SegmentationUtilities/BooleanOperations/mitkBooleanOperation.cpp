/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkBooleanOperation.h"
#include <mitkExceptionMacro.h>
#include <mitkImageTimeSelector.h>

static mitk::Image::ConstPointer Get3DSegmentation(mitk::Image::ConstPointer segmentation, unsigned int time)
{
  if (segmentation->GetDimension() != 4)
    return segmentation;

  mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();

  imageTimeSelector->SetInput(segmentation);
  imageTimeSelector->SetTimeNr(static_cast<int>(time));

  imageTimeSelector->UpdateLargestPossibleRegion();

  return imageTimeSelector->GetOutput();
}

mitk::BooleanOperation::BooleanOperation(Type type, mitk::Image::ConstPointer segmentation1, mitk::Image::ConstPointer segmentation2, unsigned int time)
  : m_Type(type),
    m_Segmentation1(segmentation1),
    m_Segmentation2(segmentation2),
    m_Time(time)
{
  this->ValidateSegmentations();
}

mitk::BooleanOperation::~BooleanOperation()
{
}

mitk::Image::Pointer mitk::BooleanOperation::GetResult() const
{
  return NULL;
}

void mitk::BooleanOperation::ValidateSegmentation(mitk::Image::ConstPointer segmentation) const
{
  if (segmentation.IsNull())
    mitkThrow() << "Segmentation is NULL!";

  if (segmentation->GetImageDescriptor()->GetNumberOfChannels() != 1)
    mitkThrow() << "Segmentation has more than one channel!";

  mitk::PixelType pixelType = segmentation->GetImageDescriptor()->GetChannelDescriptor().GetPixelType();

  if (pixelType.GetPixelType() != itk::ImageIOBase::SCALAR || pixelType.GetComponentType() != itk::ImageIOBase::UCHAR)
    mitkThrow() << "Segmentation is not a scalar image of type 'unsigned char'!";

  unsigned int dimension = segmentation->GetDimension();

  if (dimension > 4)
    mitkThrow() << "Segmentation has more than four dimensions!";

  if (m_Time != 0)
  {
    if(dimension < 4)
      mitkThrow() << "Expected four-dimensional segmentation!";

    if (segmentation->GetDimension(3) < m_Time)
      mitkThrow() << "Extend of fourth dimension of segmentation is less than specified time!";
  }
  else if (dimension < 3)
  {
    mitkThrow() << "Segmentation has less than three dimensions!";
  }
}

void mitk::BooleanOperation::ValidateSegmentations() const
{
  this->ValidateSegmentation(m_Segmentation1);
  this->ValidateSegmentation(m_Segmentation2);

  if (m_Segmentation1->GetDimension() != m_Segmentation2->GetDimension())
    mitkThrow() << "Segmentations have different dimensions!";
}
