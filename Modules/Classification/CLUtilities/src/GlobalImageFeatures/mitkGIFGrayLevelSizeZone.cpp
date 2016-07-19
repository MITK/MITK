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

#include <mitkGIFGrayLevelSizeZone.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>

// ITK
#include <itkEnhancedScalarImageToSizeZoneFeaturesFilter.h>
#include <itkMinimumMaximumImageCalculator.h>

// STL
#include <sstream>

template<typename TPixel, unsigned int VImageDimension>
void
  CalculateGrayLevelSizeZoneFeatures(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, mitk::GIFGrayLevelSizeZone::FeatureListType & featureList, mitk::GIFGrayLevelSizeZone::ParameterStruct params)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<TPixel, VImageDimension> MaskType;
  typedef itk::Statistics::EnhancedScalarImageToSizeZoneFeaturesFilter<ImageType> FilterType;
  typedef itk::MinimumMaximumImageCalculator<ImageType> MinMaxComputerType;
  typedef typename FilterType::SizeZoneFeaturesFilterType TextureFilterType;

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
  requestedFeatures->push_back(TextureFilterType::SmallZoneEmphasis);
  requestedFeatures->push_back(TextureFilterType::LargeZoneEmphasis);
  requestedFeatures->push_back(TextureFilterType::GreyLevelNonuniformity);
  requestedFeatures->push_back(TextureFilterType::SizeZoneNonuniformity);
  requestedFeatures->push_back(TextureFilterType::LowGreyLevelZoneEmphasis);
  requestedFeatures->push_back(TextureFilterType::HighGreyLevelZoneEmphasis);
  requestedFeatures->push_back(TextureFilterType::SmallZoneLowGreyLevelEmphasis);
  requestedFeatures->push_back(TextureFilterType::SmallZoneHighGreyLevelEmphasis);
  requestedFeatures->push_back(TextureFilterType::LargeZoneLowGreyLevelEmphasis);
  requestedFeatures->push_back(TextureFilterType::LargeZoneHighGreyLevelEmphasis);
  requestedFeatures->push_back(TextureFilterType::ZonePercentage);
  requestedFeatures->push_back(TextureFilterType::GreyLevelVariance);
  requestedFeatures->push_back(TextureFilterType::SizeZoneVariance);
  requestedFeatures->push_back(TextureFilterType::ZoneEntropy);

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
    case TextureFilterType::SmallZoneEmphasis :
      featureList.push_back(std::make_pair("SizeZone. ("+ strRange+") SmallZoneEmphasis Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::LargeZoneEmphasis :
      featureList.push_back(std::make_pair("SizeZone. ("+ strRange+") LargeZoneEmphasis Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::GreyLevelNonuniformity :
      featureList.push_back(std::make_pair("SizeZone. ("+ strRange+") GreyLevelNonuniformity Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::SizeZoneNonuniformity :
      featureList.push_back(std::make_pair("SizeZone. ("+ strRange+") SizeZoneNonuniformity Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::LowGreyLevelZoneEmphasis :
      featureList.push_back(std::make_pair("SizeZone. ("+ strRange+") LowGreyLevelZoneEmphasis Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::HighGreyLevelZoneEmphasis :
      featureList.push_back(std::make_pair("SizeZone. ("+ strRange+") HighGreyLevelZoneEmphasis Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::SmallZoneLowGreyLevelEmphasis :
      featureList.push_back(std::make_pair("SizeZone. ("+ strRange+") SmallZoneLowGreyLevelEmphasis Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::SmallZoneHighGreyLevelEmphasis :
      featureList.push_back(std::make_pair("SizeZone. ("+ strRange+") SmallZoneHighGreyLevelEmphasis Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::LargeZoneLowGreyLevelEmphasis :
      featureList.push_back(std::make_pair("SizeZone. ("+ strRange+") LargeZoneLowGreyLevelEmphasis Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::LargeZoneHighGreyLevelEmphasis :
      featureList.push_back(std::make_pair("SizeZone. ("+ strRange+") LargeZoneHighGreyLevelEmphasis Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::ZonePercentage :
      featureList.push_back(std::make_pair("SizeZone. ("+ strRange+") ZonePercentage Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::GreyLevelVariance :
      featureList.push_back(std::make_pair("SizeZone. ("+ strRange+") GreyLevelVariance Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::SizeZoneVariance :
      featureList.push_back(std::make_pair("SizeZone. ("+ strRange+") SizeZoneVariance Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::ZoneEntropy :
      featureList.push_back(std::make_pair("SizeZone. ("+ strRange+") ZoneEntropy Means",featureMeans->ElementAt(i)));
      break;
    default:
      break;
    }
  }
}

mitk::GIFGrayLevelSizeZone::GIFGrayLevelSizeZone():
m_Range(1.0), m_UseCtRange(false), m_Direction(0)
{
}

mitk::GIFGrayLevelSizeZone::FeatureListType mitk::GIFGrayLevelSizeZone::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;

  ParameterStruct params;
  params.m_UseCtRange=m_UseCtRange;
  params.m_Range = m_Range;
  params.m_Direction = m_Direction;

  AccessByItk_3(image, CalculateGrayLevelSizeZoneFeatures, mask, featureList,params);

  return featureList;
}

mitk::GIFGrayLevelSizeZone::FeatureNameListType mitk::GIFGrayLevelSizeZone::GetFeatureNames()
{
  FeatureNameListType featureList;
  featureList.push_back("SizeZone. SmallZoneEmphasis Means");
  featureList.push_back("SizeZone. SmallZoneEmphasis Std.");
  featureList.push_back("SizeZone. LargeZoneEmphasis Means");
  featureList.push_back("SizeZone. LargeZoneEmphasis Std.");
  featureList.push_back("SizeZone. GreyLevelNonuniformity Means");
  featureList.push_back("SizeZone. GreyLevelNonuniformity Std.");
  featureList.push_back("SizeZone. SizeZoneNonuniformity Means");
  featureList.push_back("SizeZone. SizeZoneNonuniformity Std.");
  featureList.push_back("SizeZone. LowGreyLevelZoneEmphasis Means");
  featureList.push_back("SizeZone. LowGreyLevelZoneEmphasis Std.");
  featureList.push_back("SizeZone. HighGreyLevelZoneEmphasis Means");
  featureList.push_back("SizeZone. HighGreyLevelZoneEmphasis Std.");
  featureList.push_back("SizeZone. SmallZoneLowGreyLevelEmphasis Means");
  featureList.push_back("SizeZone. SmallZoneLowGreyLevelEmphasis Std.");
  featureList.push_back("SizeZone. SmallZoneHighGreyLevelEmphasis Means");
  featureList.push_back("SizeZone. SmallZoneHighGreyLevelEmphasis Std.");
  featureList.push_back("SizeZone. LargeZoneHighGreyLevelEmphasis Means");
  featureList.push_back("SizeZone. LargeZoneHighGreyLevelEmphasis Std.");
  return featureList;
}
