/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMorphologicalOperations.h"
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryCrossStructuringElement.h>
#include <itkBinaryDilateImageFilter.h>
#include <itkBinaryErodeImageFilter.h>
#include <itkBinaryFillholeImageFilter.h>
#include <itkBinaryMorphologicalClosingImageFilter.h>
#include <itkBinaryMorphologicalOpeningImageFilter.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageTimeSelector.h>

void mitk::MorphologicalOperations::Closing(mitk::Image::Pointer &image,
                                            int factor,
                                            mitk::MorphologicalOperations::StructuralElementType structuralElement)
{
  MITK_INFO << "Start Closing...";

  auto timeSteps = static_cast<int>(image->GetTimeSteps());

  if (timeSteps > 1)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(image);

    for (int t = 0; t < timeSteps; ++t)
    {
      MITK_INFO << "  Processing time step " << t;

      timeSelector->SetTimeNr(t);
      timeSelector->Update();

      mitk::Image::Pointer img3D = timeSelector->GetOutput();
      img3D->DisconnectPipeline();

      AccessByItk_3(img3D, itkClosing, img3D, factor, structuralElement);

      mitk::ImageReadAccessor accessor(img3D);
      image->SetVolume(accessor.GetData(), t);
    }
  }
  else
  {
    AccessByItk_3(image, itkClosing, image, factor, structuralElement);
  }

  MITK_INFO << "Finished Closing";
}

void mitk::MorphologicalOperations::Erode(mitk::Image::Pointer &image,
                                          int factor,
                                          mitk::MorphologicalOperations::StructuralElementType structuralElement)
{
  MITK_INFO << "Start Erode...";

  auto timeSteps = static_cast<int>(image->GetTimeSteps());

  if (timeSteps > 1)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(image);

    for (int t = 0; t < timeSteps; ++t)
    {
      MITK_INFO << "  Processing time step " << t;

      timeSelector->SetTimeNr(t);
      timeSelector->Update();

      mitk::Image::Pointer img3D = timeSelector->GetOutput();
      img3D->DisconnectPipeline();

      AccessByItk_3(img3D, itkErode, img3D, factor, structuralElement);

      mitk::ImageReadAccessor accessor(img3D);
      image->SetVolume(accessor.GetData(), t);
    }
  }
  else
  {
    AccessByItk_3(image, itkErode, image, factor, structuralElement);
  }

  MITK_INFO << "Finished Erode";
}

void mitk::MorphologicalOperations::Dilate(mitk::Image::Pointer &image,
                                           int factor,
                                           mitk::MorphologicalOperations::StructuralElementType structuralElement)
{
  MITK_INFO << "Start Dilate...";

  auto timeSteps = static_cast<int>(image->GetTimeSteps());

  if (timeSteps > 1)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(image);

    for (int t = 0; t < timeSteps; ++t)
    {
      MITK_INFO << "  Processing time step " << t;

      timeSelector->SetTimeNr(t);
      timeSelector->Update();

      mitk::Image::Pointer img3D = timeSelector->GetOutput();
      img3D->DisconnectPipeline();

      AccessByItk_3(img3D, itkDilate, img3D, factor, structuralElement);

      mitk::ImageReadAccessor accessor(img3D);
      image->SetVolume(accessor.GetData(), t);
    }
  }
  else
  {
    AccessByItk_3(image, itkDilate, image, factor, structuralElement);
  }

  MITK_INFO << "Finished Dilate";
}

void mitk::MorphologicalOperations::Opening(mitk::Image::Pointer &image,
                                            int factor,
                                            mitk::MorphologicalOperations::StructuralElementType structuralElement)
{
  MITK_INFO << "Start Opening...";

  auto timeSteps = static_cast<int>(image->GetTimeSteps());

  if (timeSteps > 1)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(image);

    for (int t = 0; t < timeSteps; ++t)
    {
      MITK_INFO << "  Processing time step " << t;

      timeSelector->SetTimeNr(t);
      timeSelector->Update();

      mitk::Image::Pointer img3D = timeSelector->GetOutput();
      img3D->DisconnectPipeline();

      AccessByItk_3(img3D, itkOpening, img3D, factor, structuralElement);

      mitk::ImageReadAccessor accessor(img3D);
      image->SetVolume(accessor.GetData(), t);
    }
  }
  else
  {
    AccessByItk_3(image, itkOpening, image, factor, structuralElement);
  }

  MITK_INFO << "Finished Opening";
}

void mitk::MorphologicalOperations::FillHoles(mitk::Image::Pointer &image)
{
  MITK_INFO << "Start FillHole...";

  auto timeSteps = static_cast<int>(image->GetTimeSteps());

  if (timeSteps > 1)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(image);

    for (int t = 0; t < timeSteps; ++t)
    {
      MITK_INFO << "  Processing time step " << t;

      timeSelector->SetTimeNr(t);
      timeSelector->Update();

      mitk::Image::Pointer img3D = timeSelector->GetOutput();
      img3D->DisconnectPipeline();

      AccessByItk_1(img3D, itkFillHoles, img3D);

      mitk::ImageReadAccessor accessor(img3D);
      image->SetVolume(accessor.GetData(), t);
    }
  }
  else
  {
    AccessByItk_1(image, itkFillHoles, image);
  }

  MITK_INFO << "Finished FillHole";
}

template <typename TPixel, unsigned int VDimension>
void mitk::MorphologicalOperations::itkClosing(
  itk::Image<TPixel, VDimension> *sourceImage,
  mitk::Image::Pointer &resultImage,
  int factor,
  mitk::MorphologicalOperations::StructuralElementType structuralElementFlags)
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef itk::BinaryBallStructuringElement<TPixel, VDimension> BallType;
  typedef itk::BinaryCrossStructuringElement<TPixel, VDimension> CrossType;
  typedef typename itk::BinaryMorphologicalClosingImageFilter<ImageType, ImageType, BallType> BallClosingFilterType;
  typedef typename itk::BinaryMorphologicalClosingImageFilter<ImageType, ImageType, CrossType> CrossClosingFilterType;

  if (structuralElementFlags & (Ball_Axial | Ball_Coronal | Ball_Sagital))
  {
    BallType ball = CreateStructuringElement<BallType>(structuralElementFlags, factor);

    typename BallClosingFilterType::Pointer closingFilter = BallClosingFilterType::New();
    closingFilter->SetKernel(ball);
    closingFilter->SetInput(sourceImage);
    closingFilter->SetForegroundValue(1);
    closingFilter->UpdateLargestPossibleRegion();

    mitk::CastToMitkImage(closingFilter->GetOutput(), resultImage);
  }
  else
  {
    CrossType cross = CreateStructuringElement<CrossType>(structuralElementFlags, factor);

    typename CrossClosingFilterType::Pointer closingFilter = CrossClosingFilterType::New();
    closingFilter->SetKernel(cross);
    closingFilter->SetInput(sourceImage);
    closingFilter->SetForegroundValue(1);
    closingFilter->UpdateLargestPossibleRegion();

    mitk::CastToMitkImage(closingFilter->GetOutput(), resultImage);
  }
}

template <typename TPixel, unsigned int VDimension>
void mitk::MorphologicalOperations::itkErode(
  itk::Image<TPixel, VDimension> *sourceImage,
  mitk::Image::Pointer &resultImage,
  int factor,
  mitk::MorphologicalOperations::StructuralElementType structuralElementFlags)
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef itk::BinaryBallStructuringElement<TPixel, VDimension> BallType;
  typedef itk::BinaryCrossStructuringElement<TPixel, VDimension> CrossType;
  typedef typename itk::BinaryErodeImageFilter<ImageType, ImageType, BallType> BallErodeFilterType;
  typedef typename itk::BinaryErodeImageFilter<ImageType, ImageType, CrossType> CrossErodeFilterType;

  if (structuralElementFlags & (Ball_Axial | Ball_Coronal | Ball_Sagital))
  {
    BallType ball = CreateStructuringElement<BallType>(structuralElementFlags, factor);

    typename BallErodeFilterType::Pointer erodeFilter = BallErodeFilterType::New();
    erodeFilter->SetKernel(ball);
    erodeFilter->SetInput(sourceImage);
    erodeFilter->SetErodeValue(1);
    erodeFilter->UpdateLargestPossibleRegion();

    mitk::CastToMitkImage(erodeFilter->GetOutput(), resultImage);
  }
  else
  {
    CrossType cross = CreateStructuringElement<CrossType>(structuralElementFlags, factor);

    typename CrossErodeFilterType::Pointer erodeFilter = CrossErodeFilterType::New();
    erodeFilter->SetKernel(cross);
    erodeFilter->SetInput(sourceImage);
    erodeFilter->SetErodeValue(1);
    erodeFilter->UpdateLargestPossibleRegion();

    mitk::CastToMitkImage(erodeFilter->GetOutput(), resultImage);
  }
}

template <typename TPixel, unsigned int VDimension>
void mitk::MorphologicalOperations::itkDilate(
  itk::Image<TPixel, VDimension> *sourceImage,
  mitk::Image::Pointer &resultImage,
  int factor,
  mitk::MorphologicalOperations::StructuralElementType structuralElementFlags)
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef itk::BinaryBallStructuringElement<TPixel, VDimension> BallType;
  typedef itk::BinaryCrossStructuringElement<TPixel, VDimension> CrossType;
  typedef typename itk::BinaryDilateImageFilter<ImageType, ImageType, BallType> BallDilateFilterType;
  typedef typename itk::BinaryDilateImageFilter<ImageType, ImageType, CrossType> CrossDilateFilterType;

  if (structuralElementFlags & (Ball_Axial | Ball_Coronal | Ball_Sagital))
  {
    BallType ball = CreateStructuringElement<BallType>(structuralElementFlags, factor);

    typename BallDilateFilterType::Pointer dilateFilter = BallDilateFilterType::New();
    dilateFilter->SetKernel(ball);
    dilateFilter->SetInput(sourceImage);
    dilateFilter->SetDilateValue(1);
    dilateFilter->UpdateLargestPossibleRegion();

    mitk::CastToMitkImage(dilateFilter->GetOutput(), resultImage);
  }
  else
  {
    CrossType cross = CreateStructuringElement<CrossType>(structuralElementFlags, factor);

    typename CrossDilateFilterType::Pointer dilateFilter = CrossDilateFilterType::New();
    dilateFilter->SetKernel(cross);
    dilateFilter->SetInput(sourceImage);
    dilateFilter->SetDilateValue(1);
    dilateFilter->UpdateLargestPossibleRegion();

    mitk::CastToMitkImage(dilateFilter->GetOutput(), resultImage);
  }
}

template <typename TPixel, unsigned int VDimension>
void mitk::MorphologicalOperations::itkOpening(
  itk::Image<TPixel, VDimension> *sourceImage,
  mitk::Image::Pointer &resultImage,
  int factor,
  mitk::MorphologicalOperations::StructuralElementType structuralElementFlags)
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef itk::BinaryBallStructuringElement<TPixel, VDimension> BallType;
  typedef itk::BinaryCrossStructuringElement<TPixel, VDimension> CrossType;
  typedef typename itk::BinaryMorphologicalOpeningImageFilter<ImageType, ImageType, BallType> BallOpeningFiltertype;
  typedef typename itk::BinaryMorphologicalOpeningImageFilter<ImageType, ImageType, CrossType> CrossOpeningFiltertype;

  if (structuralElementFlags & (Ball_Axial | Ball_Coronal | Ball_Sagital))
  {
    BallType ball = CreateStructuringElement<BallType>(structuralElementFlags, factor);

    typename BallOpeningFiltertype::Pointer openingFilter = BallOpeningFiltertype::New();
    openingFilter->SetKernel(ball);
    openingFilter->SetInput(sourceImage);
    openingFilter->SetForegroundValue(1);
    openingFilter->SetBackgroundValue(0);
    openingFilter->UpdateLargestPossibleRegion();

    mitk::CastToMitkImage(openingFilter->GetOutput(), resultImage);
  }
  else
  {
    CrossType cross = CreateStructuringElement<CrossType>(structuralElementFlags, factor);

    typename CrossOpeningFiltertype::Pointer openingFilter = CrossOpeningFiltertype::New();
    openingFilter->SetKernel(cross);
    openingFilter->SetInput(sourceImage);
    openingFilter->SetForegroundValue(1);
    openingFilter->SetBackgroundValue(0);
    openingFilter->UpdateLargestPossibleRegion();

    mitk::CastToMitkImage(openingFilter->GetOutput(), resultImage);
  }
}

template <typename TPixel, unsigned int VDimension>
void mitk::MorphologicalOperations::itkFillHoles(itk::Image<TPixel, VDimension> *sourceImage,
                                                 mitk::Image::Pointer &resultImage)
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef typename itk::BinaryFillholeImageFilter<ImageType> FillHoleFilterType;

  typename FillHoleFilterType::Pointer fillHoleFilter = FillHoleFilterType::New();
  fillHoleFilter->SetInput(sourceImage);
  fillHoleFilter->SetForegroundValue(1);
  fillHoleFilter->UpdateLargestPossibleRegion();

  mitk::CastToMitkImage(fillHoleFilter->GetOutput(), resultImage);
}

template <class TStructuringElement>
TStructuringElement  mitk::MorphologicalOperations::CreateStructuringElement(StructuralElementType structuralElementFlag, int factor)
{
  TStructuringElement strElem;
  typename TStructuringElement::SizeType size;
  size.Fill(0);
  switch (structuralElementFlag)
  {
  case Ball_Axial:
  case Cross_Axial:
    size.SetElement(0, factor);
    size.SetElement(1, factor);
    break;
  case Ball_Coronal:
  case Cross_Coronal:
    size.SetElement(0, factor);
    size.SetElement(2, factor);
    break;
  case Ball_Sagital:
  case Cross_Sagital:
    size.SetElement(1, factor);
    size.SetElement(2, factor);
    break;
  case Ball:
  case Cross:
    size.Fill(factor);
    break;
  }

  strElem.SetRadius(size);
  strElem.CreateStructuringElement();
  return strElem;
}
