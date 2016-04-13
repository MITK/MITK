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

#include <mitkGIFGrayLevelRunLength.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>

// ITK
#include <itkEnhancedScalarImageToRunLengthFeaturesFilter.h>
#include <itkMinimumMaximumImageCalculator.h>

// STL
#include <sstream>

template<typename TPixel, unsigned int VImageDimension>
void
  CalculateGrayLevelRunLengthFeatures(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, mitk::GIFGrayLevelRunLength::FeatureListType & featureList, mitk::GIFGrayLevelRunLength::ParameterStruct params)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<TPixel, VImageDimension> MaskType;
  typedef itk::Statistics::EnhancedScalarImageToRunLengthFeaturesFilter<ImageType> FilterType;
  typedef itk::MinimumMaximumImageCalculator<ImageType> MinMaxComputerType;
  typedef typename FilterType::RunLengthFeaturesFilterType TextureFilterType;

  typename MaskType::Pointer maskImage = MaskType::New();
  mitk::CastToItkImage(mask, maskImage);

  typename FilterType::Pointer filter = FilterType::New();

  typename FilterType::OffsetVector::Pointer newOffset = FilterType::OffsetVector::New();
  auto oldOffsets = filter->GetOffsets();
  auto oldOffsetsIterator = oldOffsets->Begin();
  while (oldOffsetsIterator != oldOffsets->End())
  {
    bool continueOuterLoop = false;
    typename FilterType::OffsetType offset = oldOffsetsIterator->Value();
    for (unsigned int i = 0; i < VImageDimension; ++i)
    {
      if (params.m_Direction == i + 2 && offset[i] != 0)
      {
        continueOuterLoop = true;
      }
    }
    if (params.m_Direction == 1)
    {
      offset[0] = 0;
      offset[1] = 0;
      offset[2] = 1;
      newOffset->push_back(offset);
      break;
    }

    oldOffsetsIterator++;
    if (continueOuterLoop)
      continue;
    newOffset->push_back(offset);
  }
  filter->SetOffsets(newOffset);


  // All features are required
  typename FilterType::FeatureNameVectorPointer requestedFeatures = FilterType::FeatureNameVector::New();
  requestedFeatures->push_back(TextureFilterType::ShortRunEmphasis);
  requestedFeatures->push_back(TextureFilterType::LongRunEmphasis);
  requestedFeatures->push_back(TextureFilterType::GreyLevelNonuniformity);
  requestedFeatures->push_back(TextureFilterType::RunLengthNonuniformity);
  requestedFeatures->push_back(TextureFilterType::LowGreyLevelRunEmphasis);
  requestedFeatures->push_back(TextureFilterType::HighGreyLevelRunEmphasis);
  requestedFeatures->push_back(TextureFilterType::ShortRunLowGreyLevelEmphasis);
  requestedFeatures->push_back(TextureFilterType::ShortRunHighGreyLevelEmphasis);
  requestedFeatures->push_back(TextureFilterType::LongRunLowGreyLevelEmphasis);
  requestedFeatures->push_back(TextureFilterType::LongRunHighGreyLevelEmphasis);
  requestedFeatures->push_back(TextureFilterType::RunPercentage);
  requestedFeatures->push_back(TextureFilterType::NumberOfRuns);

  typename MinMaxComputerType::Pointer minMaxComputer = MinMaxComputerType::New();
  minMaxComputer->SetImage(itkImage);
  minMaxComputer->Compute();

  filter->SetInput(itkImage);
  filter->SetMaskImage(maskImage);
  filter->SetRequestedFeatures(requestedFeatures);
  int rangeOfPixels = params.m_Range;
  if (rangeOfPixels < 2)
    rangeOfPixels = 256;

  if (params.m_UseCtRange)
  {
    filter->SetPixelValueMinMax((TPixel)(-1024.5),(TPixel)(3096.5));
    filter->SetNumberOfBinsPerAxis(3096.5+1024.5);
  } else
  {
    filter->SetPixelValueMinMax(minMaxComputer->GetMinimum(),minMaxComputer->GetMaximum());
    filter->SetNumberOfBinsPerAxis(rangeOfPixels);
  }

  filter->SetDistanceValueMinMax(0,rangeOfPixels);

  filter->Update();

  auto featureMeans = filter->GetFeatureMeans ();
  auto featureStd = filter->GetFeatureStandardDeviations();

  std::ostringstream  ss;
  ss << rangeOfPixels;
  std::string strRange = ss.str();
  for (std::size_t i = 0; i < featureMeans->size(); ++i)
  {
    switch (i)
    {
    case TextureFilterType::ShortRunEmphasis :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") ShortRunEmphasis Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") ShortRunEmphasis Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::LongRunEmphasis :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") LongRunEmphasis Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") LongRunEmphasis Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::GreyLevelNonuniformity :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") GreyLevelNonuniformity Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") GreyLevelNonuniformity Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::RunLengthNonuniformity :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") RunLengthNonuniformity Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") RunLengthNonuniformity Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::LowGreyLevelRunEmphasis :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") LowGreyLevelRunEmphasis Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") LowGreyLevelRunEmphasis Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::HighGreyLevelRunEmphasis :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") HighGreyLevelRunEmphasis Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") HighGreyLevelRunEmphasis Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::ShortRunLowGreyLevelEmphasis :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") ShortRunLowGreyLevelEmphasis Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") ShortRunLowGreyLevelEmphasis Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::ShortRunHighGreyLevelEmphasis :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") ShortRunHighGreyLevelEmphasis Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") ShortRunHighGreyLevelEmphasis Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::LongRunLowGreyLevelEmphasis :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") LongRunLowGreyLevelEmphasis Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") LongRunLowGreyLevelEmphasis Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::LongRunHighGreyLevelEmphasis :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") LongRunHighGreyLevelEmphasis Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") LongRunHighGreyLevelEmphasis Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::RunPercentage :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") RunPercentage Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") RunPercentage Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::NumberOfRuns :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") NumberOfRuns Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") NumberOfRuns Std.",featureStd->ElementAt(i)));
      break;
    default:
      break;
    }
  }
}

mitk::GIFGrayLevelRunLength::GIFGrayLevelRunLength():
m_Range(1.0), m_UseCtRange(false), m_Direction(0)
{
}

mitk::GIFGrayLevelRunLength::FeatureListType mitk::GIFGrayLevelRunLength::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;

  ParameterStruct params;
  params.m_UseCtRange=m_UseCtRange;
  params.m_Range = m_Range;
  params.m_Direction = m_Direction;

  AccessByItk_3(image, CalculateGrayLevelRunLengthFeatures, mask, featureList,params);

  return featureList;
}

mitk::GIFGrayLevelRunLength::FeatureNameListType mitk::GIFGrayLevelRunLength::GetFeatureNames()
{
  FeatureNameListType featureList;
  featureList.push_back("RunLength. ShortRunEmphasis Means");
  featureList.push_back("RunLength. ShortRunEmphasis Std.");
  featureList.push_back("RunLength. LongRunEmphasis Means");
  featureList.push_back("RunLength. LongRunEmphasis Std.");
  featureList.push_back("RunLength. GreyLevelNonuniformity Means");
  featureList.push_back("RunLength. GreyLevelNonuniformity Std.");
  featureList.push_back("RunLength. RunLengthNonuniformity Means");
  featureList.push_back("RunLength. RunLengthNonuniformity Std.");
  featureList.push_back("RunLength. LowGreyLevelRunEmphasis Means");
  featureList.push_back("RunLength. LowGreyLevelRunEmphasis Std.");
  featureList.push_back("RunLength. HighGreyLevelRunEmphasis Means");
  featureList.push_back("RunLength. HighGreyLevelRunEmphasis Std.");
  featureList.push_back("RunLength. ShortRunLowGreyLevelEmphasis Means");
  featureList.push_back("RunLength. ShortRunLowGreyLevelEmphasis Std.");
  featureList.push_back("RunLength. ShortRunHighGreyLevelEmphasis Means");
  featureList.push_back("RunLength. ShortRunHighGreyLevelEmphasis Std.");
  featureList.push_back("RunLength. LongRunHighGreyLevelEmphasis Means");
  featureList.push_back("RunLength. LongRunHighGreyLevelEmphasis Std.");
  return featureList;
}