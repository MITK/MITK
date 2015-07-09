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

#include <mitkGIFFirstOrderStatistics.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>

// ITK
#include <itkLabelStatisticsImageFilter.h>

// STL
#include <sstream>

template<typename TPixel, unsigned int VImageDimension>
void
  CalculateFirstOrderStatistics(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, mitk::GIFFirstOrderStatistics::FeatureListType & featureList)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<int, VImageDimension> MaskType;
  typedef itk::LabelStatisticsImageFilter<ImageType, MaskType> FilterType;

  typename MaskType::Pointer maskImage = MaskType::New();
  mitk::CastToItkImage(mask, maskImage);

  typename FilterType::Pointer labelStatisticsImageFilter = FilterType::New();
  labelStatisticsImageFilter->SetInput( itkImage );
  labelStatisticsImageFilter->SetLabelInput(maskImage);
  labelStatisticsImageFilter->Update();

  featureList.push_back(std::make_pair("FirstOrder Minimum",labelStatisticsImageFilter->GetMinimum(1)));
  featureList.push_back(std::make_pair("FirstOrder Maximum ",labelStatisticsImageFilter->GetMaximum(1)));
  featureList.push_back(std::make_pair("FirstOrder Mean",labelStatisticsImageFilter->GetMean(1)));
  featureList.push_back(std::make_pair("FirstOrder Variance",labelStatisticsImageFilter->GetVariance(1)));
  featureList.push_back(std::make_pair("FirstOrder Sum",labelStatisticsImageFilter->GetSum(1)));
  featureList.push_back(std::make_pair("FirstOrder Median",labelStatisticsImageFilter->GetMedian(1)));
  featureList.push_back(std::make_pair("FirstOrder Sigma",labelStatisticsImageFilter->GetSigma(1)));
  featureList.push_back(std::make_pair("FirstOrder No. of Voxel",labelStatisticsImageFilter->GetCount(1)));
}

mitk::GIFFirstOrderStatistics::GIFFirstOrderStatistics()
{
}

mitk::GIFFirstOrderStatistics::FeatureListType mitk::GIFFirstOrderStatistics::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;

  AccessByItk_2(image, CalculateFirstOrderStatistics, mask, featureList);

  return featureList;
}

mitk::GIFFirstOrderStatistics::FeatureNameListType mitk::GIFFirstOrderStatistics::GetFeatureNames()
{
  FeatureNameListType featureList;
  featureList.push_back("FirstOrder Minimum");
  featureList.push_back("FirstOrder Maximum");
  featureList.push_back("FirstOrder Mean");
  featureList.push_back("FirstOrder Variance");
  featureList.push_back("FirstOrder Sum");
  featureList.push_back("FirstOrder Median");
  featureList.push_back("FirstOrder Sigma");
  featureList.push_back("FirstOrder No. of Voxel");
  return featureList;
}