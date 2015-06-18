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

#include "mitkFeatureBasedEdgeDetectionFilter.h"

#include <itkBinaryThresholdImageFilter.h>
#include <itkImage.h>

#include <SegmentationUtilities/MorphologicalOperations/mitkMorphologicalOperations.h>
#include <mitkUnstructuredGrid.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageStatisticsCalculator.h>
#include <mitkImage.h>
#include <mitkITKImageImport.h>
#include <mitkMaskImageFilter.h>
#include <mitkImageToPointCloudFilter.h>
#include <mitkUnstructuredGrid.h>
#include <mitkProgressBar.h>

#include <mitkIOUtil.h>
#include <mitkBaseData.h>
#include <itkNeighborhoodIterator.h>

mitk::FeatureBasedEdgeDetectionFilter::FeatureBasedEdgeDetectionFilter()
{
  m_PointGrid = mitk::UnstructuredGrid::New();
  m_SegmentationMask = mitk::Image::New();
  m_thresholdImage = mitk::Image::New();
  m_TestImage = mitk::Image::New();

  this->SetNumberOfRequiredInputs(1);

  this->SetNumberOfIndexedOutputs(1);
}

mitk::FeatureBasedEdgeDetectionFilter::~FeatureBasedEdgeDetectionFilter(){}

void mitk::FeatureBasedEdgeDetectionFilter::GenerateData()
{
  mitk::Image::ConstPointer image = ImageToUnstructuredGridFilter::GetInput();
  mitk::Image::Pointer ncImage = const_cast<mitk::Image*>(image.GetPointer());

  //statistics
  mitk::ImageStatisticsCalculator::Pointer statCalc = mitk::ImageStatisticsCalculator::New();
  statCalc->SetImage(image);
  statCalc->SetMaskingModeToImage();
  if(m_SegmentationMask->IsEmpty())
  {
    MITK_WARN << "Please set a segmentation mask first" << std::endl;
    return;
  }
  statCalc->SetImageMask(m_SegmentationMask);
  statCalc->ComputeStatistics();

  mitk::ImageStatisticsCalculator::Statistics stats = statCalc->GetStatistics();
  double mean = stats.GetMean();
  double stdDev = stats.GetSigma();

  double upperThreshold = mean + stdDev;
  double lowerThreshold = mean - stdDev;

  mitk::ProgressBar::GetInstance()->Progress();

  std::cout << "Upper: " << upperThreshold << " lower: " << lowerThreshold << std::endl;

  //thresholding
  AccessByItk_2(ncImage.GetPointer(), ITKThresholding, lowerThreshold, upperThreshold)

  mitk::ProgressBar::GetInstance()->Progress();

  //fill holes
//  mitk::MorphologicalOperations::FillHoles(m_thresholdImage);

//  mitk::ProgressBar::GetInstance()->Progress();

  mitk::MorphologicalOperations::Closing(m_thresholdImage,1,mitk::MorphologicalOperations::Ball);
  mitk::MorphologicalOperations::FillHoles(m_thresholdImage);
//  mitk::MorphologicalOperations::Opening(m_thresholdImage,1,mitk::MorphologicalOperations::Ball);

//  std::cout << "TPixel: " << m_thresholdImage->GetPixelType().GetPixelTypeAsString().c_str() << " DIM: " << m_thresholdImage->GetDimension() << std::endl;

  AccessByItk_1(m_thresholdImage,ContourSearch, m_thresholdImage->GetGeometry())

//  unsigned int numberOfPixels = m_thresholdImage->GetDimensions()[0] + m_thresholdImage->GetDimensions()[1] + m_thresholdImage->GetDimensions()[2];
//  for(unsigned int i=0; i<numberOfPixels; i++)
//  {
//    std::cout << "Pixel-Value at " << i << ": " << m_thresholdImage-
//  }

//  //masking
//  mitk::MaskImageFilter::Pointer maskFilter = mitk::MaskImageFilter::New();
//  maskFilter->SetInput(image);
//  maskFilter->SetMask(m_thresholdImage);
//  maskFilter->OverrideOutsideValueOn();
//  maskFilter->SetOutsideValue(0);
//  try
//  {
//    maskFilter->Update();
//  }
//  catch(itk::ExceptionObject& excpt)
//  {
//    MITK_ERROR << excpt.GetDescription();
//    return;
//  }

//  mitk::Image::Pointer resultImage = maskFilter->GetOutput();

//  mitk::ProgressBar::GetInstance()->Progress();

//  //imagetopointcloudfilter
//  mitk::ImageToPointCloudFilter::Pointer pclFilter = mitk::ImageToPointCloudFilter::New();
//  pclFilter->SetInput(resultImage);
//  pclFilter->Update();

//  mitk::ProgressBar::GetInstance()->Progress();

//  mitk::UnstructuredGrid::Pointer outp = mitk::UnstructuredGrid::New();
//  outp->SetVtkUnstructuredGrid( pclFilter->GetOutput()->GetVtkUnstructuredGrid() );
//  this->SetNthOutput(0, outp);

//  m_PointGrid->SetVtkUnstructuredGrid( pclFilter->GetOutput()->GetVtkUnstructuredGrid() );
//  m_PointGrid = this->GetOutput();
}

#include <itkSimpleContourExtractorImageFilter.h>
#include <mitkImageCast.h>

template <typename TPixel, unsigned int VImageDimension>
void mitk::FeatureBasedEdgeDetectionFilter::ContourSearch( itk::Image<TPixel, VImageDimension>* originalImage, mitk::BaseGeometry* geo)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::BinaryContourImageFilter<ImageType, ImageType> binaryContourImageFilterType;
  typedef unsigned char OutputPixelType;
  typedef itk::Image< OutputPixelType, VImageDimension > OutputImageType;

  typename binaryContourImageFilterType::Pointer binaryContourFilter = binaryContourImageFilterType::New();
  binaryContourFilter->SetInput(originalImage);
  binaryContourFilter->SetForegroundValue(1);
  binaryContourFilter->SetBackgroundValue(0);
  binaryContourFilter->Update();

//  typedef itk::SimpleContourExtractorImageFilter <ImageType, ImageType> SimpleContourExtractorImageFilterType;
//  typename SimpleContourExtractorImageFilterType::Pointer contourFilter = SimpleContourExtractorImageFilterType::New();
//  contourFilter->SetInput(originalImage);
//  contourFilter->Update();

  typename itk::Image<TPixel, VImageDimension>::Pointer itkImage = itk::Image<TPixel, VImageDimension>::New();
  itkImage->Graft(binaryContourFilter->GetOutput());
//  itkImage->Graft(contourFilter->GetOutput());

  mitk::CastToMitkImage(itkImage, m_TestImage);
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::FeatureBasedEdgeDetectionFilter::ITKThresholding( itk::Image<TPixel, VImageDimension>* originalImage, double lower, double upper)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned char, VImageDimension> SegmentationType;
  typedef itk::BinaryThresholdImageFilter<ImageType, SegmentationType> ThresholdFilterType;

  typename ThresholdFilterType::Pointer filter = ThresholdFilterType::New();
  filter->SetInput(originalImage);
  filter->SetLowerThreshold(lower);
  filter->SetUpperThreshold(upper);
  filter->SetInsideValue(1);
  filter->SetOutsideValue(0);
  filter->Update();

  mitk::CastToMitkImage(filter->GetOutput(),m_thresholdImage);
}

void mitk::FeatureBasedEdgeDetectionFilter::SetSegmentationMask(mitk::Image::Pointer segmentation)
{
  this->m_SegmentationMask = segmentation;
}

void mitk::FeatureBasedEdgeDetectionFilter::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
}
