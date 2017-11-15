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

#include <mitkGIFImageDescriptionFeatures.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>

// ITK
#include <itkLabelStatisticsImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkImageRegionConstIteratorWithIndex.h>

// STL
#include <sstream>

template<typename TPixel, unsigned int VImageDimension>
void
CalculateFirstOrderStatistics(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, mitk::GIFImageDescriptionFeatures::FeatureListType & featureList, mitk::GIFImageDescriptionFeatures::ParameterStruct params)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<int, VImageDimension> MaskType;
  typedef itk::LabelStatisticsImageFilter<ImageType, MaskType> FilterType;
  typedef typename FilterType::HistogramType HistogramType;
  typedef typename HistogramType::IndexType HIndexType;
  typedef itk::MinimumMaximumImageCalculator<ImageType> MinMaxComputerType;

  typename MaskType::Pointer maskImage = MaskType::New();
  mitk::CastToItkImage(mask, maskImage);

  unsigned int imageDimensionX = itkImage->GetLargestPossibleRegion().GetSize()[0];
  unsigned int imageDimensionY = itkImage->GetLargestPossibleRegion().GetSize()[1];
  unsigned int imageDimensionZ = itkImage->GetLargestPossibleRegion().GetSize()[2];

  double imageVoxelSpacingX = itkImage->GetSpacing()[0];
  double imageVoxelSpacingY = itkImage->GetSpacing()[1];
  double imageVoxelSpacingZ = itkImage->GetSpacing()[2];

  typename MinMaxComputerType::Pointer minMaxComputer = MinMaxComputerType::New();
  minMaxComputer->SetImage(itkImage);
  minMaxComputer->Compute();

  double imageMinimum = minMaxComputer->GetMinimum();
  double imageMaximum = minMaxComputer->GetMaximum();


  unsigned int maskDimensionX = maskImage->GetLargestPossibleRegion().GetSize()[0];
  unsigned int maskDimensionY = maskImage->GetLargestPossibleRegion().GetSize()[1];
  unsigned int maskDimensionZ = maskImage->GetLargestPossibleRegion().GetSize()[2];

  double maskVoxelSpacingX = maskImage->GetSpacing()[0];
  double maskVoxelSpacingY = maskImage->GetSpacing()[1];
  double maskVoxelSpacingZ = maskImage->GetSpacing()[2];


  unsigned int voxelCount = 0;
  unsigned int maskVoxelCount = 0;
  double maskMinimum = imageMaximum;
  double maskMaximum = imageMinimum;
  double imageMean = 0;
  double maskMean = 0;

  unsigned int maskMinimumX = maskDimensionX;
  unsigned int maskMaximumX = 0;
  unsigned int maskMinimumY = maskDimensionY;
  unsigned int maskMaximumY = 0;
  unsigned int maskMinimumZ = maskDimensionZ;
  unsigned int maskMaximumZ = 0;


  itk::ImageRegionConstIteratorWithIndex<ImageType> imIter(itkImage, itkImage->GetLargestPossibleRegion());
  itk::ImageRegionConstIteratorWithIndex<MaskType> maIter(maskImage, maskImage->GetLargestPossibleRegion());

  while (!imIter.IsAtEnd())
  {
    auto pixelValue = imIter.Get();
    if (maIter.Get() > 0)
    {
      ++maskVoxelCount;
      maskMean += pixelValue;
      maskMinimum = (maskMinimum > pixelValue) ? pixelValue : maskMinimum;
      maskMaximum = (maskMaximum < pixelValue) ? pixelValue : maskMaximum;
      maskMinimumX = (maskMinimumX > imIter.GetIndex()[0]) ? imIter.GetIndex()[0] : maskMinimumX;
      maskMaximumX = (maskMaximumX < imIter.GetIndex()[0]) ? imIter.GetIndex()[0] : maskMaximumX;
      maskMinimumY = (maskMinimumY > imIter.GetIndex()[1]) ? imIter.GetIndex()[1] : maskMinimumY;
      maskMaximumY = (maskMaximumY < imIter.GetIndex()[1]) ? imIter.GetIndex()[1] : maskMaximumY;
      maskMinimumZ = (maskMinimumZ > imIter.GetIndex()[2]) ? imIter.GetIndex()[2] : maskMinimumZ;
      maskMaximumZ = (maskMaximumZ < imIter.GetIndex()[2]) ? imIter.GetIndex()[2] : maskMaximumZ;
    }
    ++voxelCount;
    imageMean += pixelValue;
    ++imIter;
    ++maIter;
  }
  imageMean /= voxelCount;
  maskMean /= maskVoxelCount;

  featureList.push_back(std::make_pair("Diagnostic (Image)::Dimension X", imageDimensionX));
  featureList.push_back(std::make_pair("Diagnostic (Image)::Dimension Y", imageDimensionY));
  featureList.push_back(std::make_pair("Diagnostic (Image)::Dimension Z", imageDimensionZ));
  featureList.push_back(std::make_pair("Diagnostic (Image)::Spacing X", imageVoxelSpacingX));
  featureList.push_back(std::make_pair("Diagnostic (Image)::Spacing Y", imageVoxelSpacingY));
  featureList.push_back(std::make_pair("Diagnostic (Image)::Spacing Z", imageVoxelSpacingZ));
  featureList.push_back(std::make_pair("Diagnostic (Image)::Mean Intensity", imageMean));
  featureList.push_back(std::make_pair("Diagnostic (Image)::Minimum Intensity", imageMinimum));
  featureList.push_back(std::make_pair("Diagnostic (Image)::Maximum Intensity", imageMaximum));

  featureList.push_back(std::make_pair("Diagnostic (Mask)::Dimension X", maskDimensionX));
  featureList.push_back(std::make_pair("Diagnostic (Mask)::Dimension Y", maskDimensionY));
  featureList.push_back(std::make_pair("Diagnostic (Mask)::Dimension Z", maskDimensionZ));
  featureList.push_back(std::make_pair("Diagnostic (Mask)::Mask Bounding Box X", maskMaximumX - maskMinimumX + 1));
  featureList.push_back(std::make_pair("Diagnostic (Mask)::Mask Bounding Box Y", maskMaximumY - maskMinimumY + 1));
  featureList.push_back(std::make_pair("Diagnostic (Mask)::Mask Bounding Box Z", maskMaximumZ - maskMinimumZ + 1));
  featureList.push_back(std::make_pair("Diagnostic (Mask)::Spacing X", maskVoxelSpacingX));
  featureList.push_back(std::make_pair("Diagnostic (Mask)::Spacing Y", maskVoxelSpacingY));
  featureList.push_back(std::make_pair("Diagnostic (Mask)::Spacing Z", maskVoxelSpacingZ));
  featureList.push_back(std::make_pair("Diagnostic (Mask)::Voxel Count ", maskVoxelCount));
  featureList.push_back(std::make_pair("Diagnostic (Mask)::Mean Intensity", maskMean));
  featureList.push_back(std::make_pair("Diagnostic (Mask)::Minimum Intensity", maskMinimum));
  featureList.push_back(std::make_pair("Diagnostic (Mask)::Maximum Intensity", maskMaximum));

}

mitk::GIFImageDescriptionFeatures::GIFImageDescriptionFeatures() :
m_HistogramSize(256), m_UseCtRange(false)
{
  SetShortName("id");
  SetLongName("image-diagnostic");
}

mitk::GIFImageDescriptionFeatures::FeatureListType mitk::GIFImageDescriptionFeatures::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;

  ParameterStruct params;
  params.m_HistogramSize = this->m_HistogramSize;
  params.m_UseCtRange = this->m_UseCtRange;

  params.MinimumIntensity = GetMinimumIntensity();
  params.MaximumIntensity = GetMaximumIntensity();
  params.UseMinimumIntensity = GetUseMinimumIntensity();
  params.UseMaximumIntensity = GetUseMaximumIntensity();
  params.Bins = GetBins();

  AccessByItk_3(image, CalculateFirstOrderStatistics, mask, featureList, params);

  return featureList;
}

mitk::GIFImageDescriptionFeatures::FeatureNameListType mitk::GIFImageDescriptionFeatures::GetFeatureNames()
{
  FeatureNameListType featureList;
  return featureList;
}


void mitk::GIFImageDescriptionFeatures::AddArguments(mitkCommandLineParser &parser)
{
  std::string name = GetOptionPrefix();
  parser.addArgument(GetLongName(), name, mitkCommandLineParser::String, "Use Volume-Statistic", "calculates volume based features", us::Any());
}

void
mitk::GIFImageDescriptionFeatures::CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &, const Image::Pointer &maskNoNAN, FeatureListType &featureList)
{
  auto parsedArgs = GetParameter();
  if (parsedArgs.count(GetLongName()))
  {
    MITK_INFO << "Start calculating first order features ....";
    auto localResults = this->CalculateFeatures(feature, maskNoNAN);
    featureList.insert(featureList.end(), localResults.begin(), localResults.end());
    MITK_INFO << "Finished calculating first order features....";
  }
}

