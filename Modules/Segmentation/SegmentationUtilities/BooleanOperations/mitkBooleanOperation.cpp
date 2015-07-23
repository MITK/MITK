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
#include <mitkImageCast.h>
#include <mitkImageTimeSelector.h>
#include <itkAndImageFilter.h>
#include <itkNotImageFilter.h>
#include <itkOrImageFilter.h>

typedef itk::Image< mitk::Label::PixelType, 3> ImageType;

static mitk::Image::Pointer Get3DSegmentation(mitk::Image::Pointer segmentation, unsigned int time)
{
  if (segmentation->GetDimension() != 4)
    return segmentation;

  mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();

  imageTimeSelector->SetInput(segmentation);
  imageTimeSelector->SetTimeNr(static_cast<int>(time));

  imageTimeSelector->UpdateLargestPossibleRegion();

  return imageTimeSelector->GetOutput();
}

static ImageType::Pointer CastTo3DItkImage(mitk::Image::Pointer segmentation, unsigned int time)
{
  ImageType::Pointer result;
  mitk::CastToItkImage(Get3DSegmentation(segmentation, time), result);
  return result;
}

mitk::BooleanOperation::BooleanOperation(Type type, mitk::Image::Pointer segmentation0, mitk::Image::Pointer segmentation1, unsigned int time)
  : m_Type(type),
    m_Segmentation0(segmentation0),
    m_Segmentation1(segmentation1),
    m_Time(time)
{
  this->ValidateSegmentations();
}

mitk::BooleanOperation::~BooleanOperation()
{
}

mitk::LabelSetImage::Pointer mitk::BooleanOperation::GetResult() const
{
  mitk::LabelSetImage::Pointer result;

  switch (m_Type)
  {
    case Difference:
      result = this->GetDifference();
      break;

    case Intersection:
      result = this->GetIntersection();
      break;

    case Union:
      result = this->GetUnion();
      break;

    default:
      mitkThrow() << "Unknown boolean operation type '" << m_Type << "'!";
  }

  return result;
}

mitk::LabelSetImage::Pointer mitk::BooleanOperation::GetDifference() const
{
  ImageType::Pointer input1 = CastTo3DItkImage(m_Segmentation0, m_Time);
  ImageType::Pointer input2 = CastTo3DItkImage(m_Segmentation1, m_Time);

  itk::NotImageFilter<ImageType, ImageType>::Pointer notFilter = itk::NotImageFilter<ImageType, ImageType>::New();
  notFilter->SetInput(input2);

  itk::AndImageFilter<ImageType, ImageType>::Pointer andFilter = itk::AndImageFilter<ImageType, ImageType>::New();
  andFilter->SetInput1(input1);
  andFilter->SetInput2(notFilter->GetOutput());

  andFilter->UpdateLargestPossibleRegion();

  Image::Pointer tempResult = Image::New();
  CastToMitkImage<ImageType>(andFilter->GetOutput(), tempResult);

  tempResult->DisconnectPipeline();

  mitk::LabelSetImage::Pointer result = mitk::LabelSetImage::New();
  result->InitializeByLabeledImage(tempResult );
  return result;
}

mitk::LabelSetImage::Pointer mitk::BooleanOperation::GetIntersection() const
{
  ImageType::Pointer input1 = CastTo3DItkImage(m_Segmentation0, m_Time);
  ImageType::Pointer input2 = CastTo3DItkImage(m_Segmentation1, m_Time);

  itk::AndImageFilter<ImageType, ImageType>::Pointer andFilter = itk::AndImageFilter<ImageType, ImageType>::New();
  andFilter->SetInput1(input1);
  andFilter->SetInput2(input2);

  andFilter->UpdateLargestPossibleRegion();

  Image::Pointer tempResult = Image::New();
  CastToMitkImage<ImageType>(andFilter->GetOutput(), tempResult);

  tempResult->DisconnectPipeline();
  mitk::LabelSetImage::Pointer result = mitk::LabelSetImage::New();
  result->InitializeByLabeledImage(tempResult);
  return result;
}

mitk::LabelSetImage::Pointer mitk::BooleanOperation::GetUnion() const
{
  ImageType::Pointer input1 = CastTo3DItkImage(m_Segmentation0, m_Time);
  ImageType::Pointer input2 = CastTo3DItkImage(m_Segmentation1, m_Time);

  itk::OrImageFilter<ImageType, ImageType>::Pointer orFilter = itk::OrImageFilter<ImageType, ImageType>::New();
  orFilter->SetInput1(input1);
  orFilter->SetInput2(input2);

  orFilter->UpdateLargestPossibleRegion();

  Image::Pointer tempResult = Image::New();
  CastToMitkImage<ImageType>(orFilter->GetOutput(), tempResult);

  tempResult->DisconnectPipeline();
  mitk::LabelSetImage::Pointer result = mitk::LabelSetImage::New();
  result->InitializeByLabeledImage(tempResult);
  return result;
}

void mitk::BooleanOperation::ValidateSegmentation(mitk::Image::Pointer segmentation) const
{
  if (segmentation.IsNull())
    mitkThrow() << "Segmentation is NULL!";

  if (segmentation->GetImageDescriptor()->GetNumberOfChannels() != 1)
    mitkThrow() << "Segmentation has more than one channel!";

  mitk::PixelType pixelType = segmentation->GetImageDescriptor()->GetChannelDescriptor().GetPixelType();

  if (pixelType.GetPixelType() != itk::ImageIOBase::SCALAR || ( pixelType.GetComponentType() != itk::ImageIOBase::UCHAR && pixelType.GetComponentType() != itk::ImageIOBase::USHORT ) )
  {
    mitkThrow() << "Segmentation is not of a supported type. Supported are scalar images of type 'unsigned char' or 'unsigned short'.";
  }

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
  this->ValidateSegmentation(m_Segmentation0);
  this->ValidateSegmentation(m_Segmentation1);

  if (m_Segmentation0->GetDimension() != m_Segmentation1->GetDimension())
    mitkThrow() << "Segmentations have different dimensions!";
}
