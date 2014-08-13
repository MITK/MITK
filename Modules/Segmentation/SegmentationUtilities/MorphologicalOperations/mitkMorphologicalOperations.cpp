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

#include "mitkMorphologicalOperations.h"
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageTimeSelector.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryCrossStructuringElement.h>
#include <itkBinaryDilateImageFilter.h>
#include <itkBinaryErodeImageFilter.h>
#include <itkBinaryFillholeImageFilter.h>
#include <itkBinaryMorphologicalClosingImageFilter.h>
#include <itkBinaryMorphologicalOpeningImageFilter.h>

void mitk::MorphologicalOperations::Closing(mitk::Image::Pointer& image, int factor, mitk::MorphologicalOperations::StructuralElementType structuralElement)
{
   MITK_INFO << "Start Closing...";

   int timeSteps = static_cast<int>(image->GetTimeSteps());

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

         AccessFixedPixelTypeByItk_3(img3D, itkClosing, (unsigned char), img3D, factor, structuralElement);

         mitk::ImageReadAccessor accessor(img3D);
         image->SetVolume(accessor.GetData(), t);
      }
   }
   else
   {
      AccessFixedPixelTypeByItk_3(image, itkClosing, (unsigned char), image, factor, structuralElement);
   }

   MITK_INFO << "Finished Closing";
}

void mitk::MorphologicalOperations::Erode(mitk::Image::Pointer& image, int factor, mitk::MorphologicalOperations::StructuralElementType structuralElement)
{
   MITK_INFO << "Start Erode...";

   int timeSteps = static_cast<int>(image->GetTimeSteps());

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

         AccessFixedPixelTypeByItk_3(img3D, itkErode, (unsigned char), img3D, factor, structuralElement);

         mitk::ImageReadAccessor accessor(img3D);
         image->SetVolume(accessor.GetData(), t);
      }
   }
   else
   {
      AccessFixedPixelTypeByItk_3(image, itkErode, (unsigned char), image, factor, structuralElement);
   }

   MITK_INFO << "Finished Erode";
}

void mitk::MorphologicalOperations::Dilate(mitk::Image::Pointer& image, int factor, mitk::MorphologicalOperations::StructuralElementType structuralElement)
{
   MITK_INFO << "Start Dilate...";

   int timeSteps = static_cast<int>(image->GetTimeSteps());

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

         AccessFixedPixelTypeByItk_3(img3D, itkDilate, (unsigned char), img3D, factor, structuralElement);

         mitk::ImageReadAccessor accessor(img3D);
         image->SetVolume(accessor.GetData(), t);
      }
   }
   else
   {
      AccessFixedPixelTypeByItk_3(image, itkDilate, (unsigned char), image, factor, structuralElement);
   }

   MITK_INFO << "Finished Dilate";
}

void mitk::MorphologicalOperations::Opening(mitk::Image::Pointer& image, int factor, mitk::MorphologicalOperations::StructuralElementType structuralElement)
{
   MITK_INFO << "Start Opening...";

   int timeSteps = static_cast<int>(image->GetTimeSteps());

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

         AccessFixedPixelTypeByItk_3(img3D, itkOpening, (unsigned char), img3D, factor, structuralElement);

         mitk::ImageReadAccessor accessor(img3D);
         image->SetVolume(accessor.GetData(), t);
      }
   }
   else
   {
      AccessFixedPixelTypeByItk_3(image, itkOpening, (unsigned char), image, factor, structuralElement);
   }

   MITK_INFO << "Finished Opening";
}

void mitk::MorphologicalOperations::FillHoles(mitk::Image::Pointer &image)
{
   MITK_INFO << "Start FillHole...";

   int timeSteps = static_cast<int>(image->GetTimeSteps());

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

         AccessFixedPixelTypeByItk_1(img3D, itkFillHoles, (unsigned char), img3D);

         mitk::ImageReadAccessor accessor(img3D);
         image->SetVolume(accessor.GetData(), t);
      }
   }
   else
   {
      AccessFixedPixelTypeByItk_1(image, itkFillHoles, (unsigned char), image);
   }

   MITK_INFO << "Finished FillHole";
}

template<typename TPixel, unsigned int VDimension>
void mitk::MorphologicalOperations::itkClosing(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer& resultImage, int factor, mitk::MorphologicalOperations::StructuralElementType structuralElement)
{
   typedef itk::Image<TPixel, VDimension> ImageType;
   typedef itk::BinaryBallStructuringElement<TPixel, VDimension> BallType;
   typedef itk::BinaryCrossStructuringElement<TPixel, VDimension> CrossType;
   typedef typename itk::BinaryMorphologicalClosingImageFilter<ImageType, ImageType, BallType> BallClosingFilterType;
   typedef typename itk::BinaryMorphologicalClosingImageFilter<ImageType, ImageType, CrossType> CrossClosingFilterType;

   if (structuralElement == Ball)
   {
      BallType ball;
      ball.SetRadius(factor);
      ball.CreateStructuringElement();

      typename BallClosingFilterType::Pointer closingFilter = BallClosingFilterType::New();
      closingFilter->SetKernel(ball);
      closingFilter->SetInput(sourceImage);
      closingFilter->SetForegroundValue(1);
      closingFilter->UpdateLargestPossibleRegion();

      mitk::CastToMitkImage(closingFilter->GetOutput(), resultImage);
   }
   else
   {
      CrossType cross;
      cross.CreateStructuringElement();

      typename CrossClosingFilterType::Pointer closingFilter = CrossClosingFilterType::New();
      closingFilter->SetKernel(cross);
      closingFilter->SetInput(sourceImage);
      closingFilter->SetForegroundValue(1);
      closingFilter->UpdateLargestPossibleRegion();

      mitk::CastToMitkImage(closingFilter->GetOutput(), resultImage);
   }
}

template<typename TPixel, unsigned int VDimension>
void mitk::MorphologicalOperations::itkErode(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer& resultImage, int factor, mitk::MorphologicalOperations::StructuralElementType structuralElement)
{
   typedef itk::Image<TPixel, VDimension> ImageType;
   typedef itk::BinaryBallStructuringElement<TPixel, VDimension> BallType;
   typedef itk::BinaryCrossStructuringElement<TPixel, VDimension> CrossType;
   typedef typename itk::BinaryErodeImageFilter<ImageType, ImageType, BallType> BallErodeFilterType;
   typedef typename itk::BinaryErodeImageFilter<ImageType, ImageType, CrossType> CrossErodeFilterType;

   if (structuralElement == Ball)
   {
      BallType ball;
      ball.SetRadius(factor);
      ball.CreateStructuringElement();

      typename BallErodeFilterType::Pointer erodeFilter = BallErodeFilterType::New();
      erodeFilter->SetKernel(ball);
      erodeFilter->SetInput(sourceImage);
      erodeFilter->SetErodeValue(1);
      erodeFilter->UpdateLargestPossibleRegion();

      mitk::CastToMitkImage(erodeFilter->GetOutput(), resultImage);
   }
   else
   {
      CrossType cross;
      cross.CreateStructuringElement();

      typename CrossErodeFilterType::Pointer erodeFilter = CrossErodeFilterType::New();
      erodeFilter->SetKernel(cross);
      erodeFilter->SetInput(sourceImage);
      erodeFilter->SetErodeValue(1);
      erodeFilter->UpdateLargestPossibleRegion();

      mitk::CastToMitkImage(erodeFilter->GetOutput(), resultImage);
   }
}

template<typename TPixel, unsigned int VDimension>
void mitk::MorphologicalOperations::itkDilate(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer& resultImage, int factor,  mitk::MorphologicalOperations::StructuralElementType structuralElement)
{
   typedef itk::Image<TPixel, VDimension> ImageType;
   typedef itk::BinaryBallStructuringElement<TPixel, VDimension> BallType;
   typedef itk::BinaryCrossStructuringElement<TPixel, VDimension> CrossType;
   typedef typename itk::BinaryDilateImageFilter<ImageType, ImageType, BallType> BallDilateFilterType;
   typedef typename itk::BinaryDilateImageFilter<ImageType, ImageType, CrossType> CrossDilateFilterType;

   if (structuralElement == Ball)
   {
      BallType ball;
      ball.SetRadius(factor);
      ball.CreateStructuringElement();

      typename BallDilateFilterType::Pointer dilateFilter = BallDilateFilterType::New();
      dilateFilter->SetKernel(ball);
      dilateFilter->SetInput(sourceImage);
      dilateFilter->SetDilateValue(1);
      dilateFilter->UpdateLargestPossibleRegion();

      mitk::CastToMitkImage(dilateFilter->GetOutput(), resultImage);
   }
   else
   {
      CrossType cross;
      cross.CreateStructuringElement();

      typename CrossDilateFilterType::Pointer dilateFilter = CrossDilateFilterType::New();

      dilateFilter->SetKernel(cross);
      dilateFilter->SetInput(sourceImage);
      dilateFilter->SetDilateValue(1);
      dilateFilter->UpdateLargestPossibleRegion();

      mitk::CastToMitkImage(dilateFilter->GetOutput(), resultImage);
   }
}

template<typename TPixel, unsigned int VDimension>
void mitk::MorphologicalOperations::itkOpening(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer& resultImage, int factor, mitk::MorphologicalOperations::StructuralElementType structuralElement)
{
   typedef itk::Image<TPixel, VDimension> ImageType;
   typedef itk::BinaryBallStructuringElement<TPixel, VDimension> BallType;
   typedef itk::BinaryCrossStructuringElement<TPixel, VDimension> CrossType;
   typedef typename itk::BinaryMorphologicalOpeningImageFilter<ImageType, ImageType, BallType> BallOpeningFiltertype;
   typedef typename itk::BinaryMorphologicalOpeningImageFilter<ImageType, ImageType, CrossType> CrossOpeningFiltertype;

   if (structuralElement == Ball)
   {
      BallType ball;
      ball.SetRadius(factor);
      ball.CreateStructuringElement();

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
      CrossType cross;
      cross.CreateStructuringElement();

      typename CrossOpeningFiltertype::Pointer openingFilter = CrossOpeningFiltertype::New();
      openingFilter->SetKernel(cross);
      openingFilter->SetInput(sourceImage);
      openingFilter->SetForegroundValue(1);
      openingFilter->SetBackgroundValue(0);
      openingFilter->UpdateLargestPossibleRegion();

      mitk::CastToMitkImage(openingFilter->GetOutput(), resultImage);
   }
}

template<typename TPixel, unsigned int VDimension>
void mitk::MorphologicalOperations::itkFillHoles(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer& resultImage)
{
   typedef itk::Image<TPixel, VDimension> ImageType;
   typedef typename itk::BinaryFillholeImageFilter<ImageType> FillHoleFilterType;

   typename FillHoleFilterType::Pointer fillHoleFilter = FillHoleFilterType::New();
   fillHoleFilter->SetInput(sourceImage);
   fillHoleFilter->SetForegroundValue(1);
   fillHoleFilter->UpdateLargestPossibleRegion();

   mitk::CastToMitkImage(fillHoleFilter->GetOutput(), resultImage);
}
