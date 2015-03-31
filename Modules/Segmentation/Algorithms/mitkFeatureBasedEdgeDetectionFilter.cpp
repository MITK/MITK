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

mitk::FeatureBasedEdgeDetectionFilter::FeatureBasedEdgeDetectionFilter()
{
  m_PointGrid = mitk::UnstructuredGrid::New();
  m_SegmentationMask = mitk::Image::New();
  m_thresholdImage = mitk::Image::New();

  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfRequiredOutputs(1);

  this->SetNthOutput(0, m_PointGrid);
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

  //thresholding
  AccessByItk_2(ncImage.GetPointer(), ITKThresholding, lowerThreshold, upperThreshold)

  //fill holes
  mitk::MorphologicalOperations::FillHoles(m_thresholdImage);

//  mitk::MorphologicalOperations::Closing(m_morphThreshold,1,mitk::MorphologicalOperations::Ball);
//  mitk::MorphologicalOperations::Opening(m_morphThreshold,1,mitk::MorphologicalOperations::Ball);

  //masking
  mitk::MaskImageFilter::Pointer maskFilter = mitk::MaskImageFilter::New();
  maskFilter->SetInput(image);
  maskFilter->SetMask(m_thresholdImage);
  maskFilter->OverrideOutsideValueOn();
  maskFilter->SetOutsideValue(0);
  try
  {
    maskFilter->Update();
  }
  catch(itk::ExceptionObject& excpt)
  {
    MITK_ERROR << excpt.GetDescription();
    return;
  }

  mitk::Image::Pointer resultImage = maskFilter->GetOutput();

  //imagetopointcloudfilter
  mitk::ImageToPointCloudFilter::Pointer pclFilter = mitk::ImageToPointCloudFilter::New();
  pclFilter->SetInput(resultImage);
  pclFilter->Update();

  m_PointGrid->SetVtkUnstructuredGrid( pclFilter->GetOutput()->GetVtkUnstructuredGrid() );
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

  m_thresholdImage = mitk::ImportItkImage(filter->GetOutput());
}

void mitk::FeatureBasedEdgeDetectionFilter::SetSegmentationMask(mitk::Image::Pointer segmentation)
{
  this->m_SegmentationMask = segmentation;
}

void mitk::FeatureBasedEdgeDetectionFilter::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
}
