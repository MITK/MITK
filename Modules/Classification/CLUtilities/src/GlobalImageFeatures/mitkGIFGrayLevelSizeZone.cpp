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
  requestedFeatures->push_back(TextureFilterType::GreyLevelNonuniformityNormalized);
  requestedFeatures->push_back(TextureFilterType::SizeZoneNonuniformity);
  requestedFeatures->push_back(TextureFilterType::SizeZoneNonuniformityNormalized);
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
      featureList.push_back(std::make_pair("SizeZone ("+ strRange+") SmallZoneEmphasis Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::LargeZoneEmphasis :
      featureList.push_back(std::make_pair("SizeZone ("+ strRange+") LargeZoneEmphasis Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::GreyLevelNonuniformity :
      featureList.push_back(std::make_pair("SizeZone ("+ strRange+") GreyLevelNonuniformity Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::GreyLevelNonuniformityNormalized :
      featureList.push_back(std::make_pair("SizeZone ("+ strRange+") GreyLevelNonuniformityNormalized Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::SizeZoneNonuniformity :
      featureList.push_back(std::make_pair("SizeZone ("+ strRange+") SizeZoneNonuniformity Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::SizeZoneNonuniformityNormalized :
      featureList.push_back(std::make_pair("SizeZone ("+ strRange+") SizeZoneNonuniformityNormalized Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::LowGreyLevelZoneEmphasis :
      featureList.push_back(std::make_pair("SizeZone ("+ strRange+") LowGreyLevelZoneEmphasis Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::HighGreyLevelZoneEmphasis :
      featureList.push_back(std::make_pair("SizeZone ("+ strRange+") HighGreyLevelZoneEmphasis Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::SmallZoneLowGreyLevelEmphasis :
      featureList.push_back(std::make_pair("SizeZone ("+ strRange+") SmallZoneLowGreyLevelEmphasis Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::SmallZoneHighGreyLevelEmphasis :
      featureList.push_back(std::make_pair("SizeZone ("+ strRange+") SmallZoneHighGreyLevelEmphasis Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::LargeZoneLowGreyLevelEmphasis :
      featureList.push_back(std::make_pair("SizeZone ("+ strRange+") LargeZoneLowGreyLevelEmphasis Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::LargeZoneHighGreyLevelEmphasis :
      featureList.push_back(std::make_pair("SizeZone ("+ strRange+") LargeZoneHighGreyLevelEmphasis Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::ZonePercentage :
      featureList.push_back(std::make_pair("SizeZone ("+ strRange+") ZonePercentage Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::GreyLevelVariance :
      featureList.push_back(std::make_pair("SizeZone ("+ strRange+") GreyLevelVariance Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::SizeZoneVariance :
      featureList.push_back(std::make_pair("SizeZone ("+ strRange+") SizeZoneVariance Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::ZoneEntropy :
      featureList.push_back(std::make_pair("SizeZone ("+ strRange+") ZoneEntropy Means",featureMeans->ElementAt(i)));
      break;
    default:
      break;
    }
  }
}

mitk::GIFGrayLevelSizeZone::GIFGrayLevelSizeZone():
m_Range(1.0), m_UseCtRange(false)
{
  SetShortName("glsz");
  SetLongName("greylevelsizezone");
}

mitk::GIFGrayLevelSizeZone::FeatureListType mitk::GIFGrayLevelSizeZone::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;

  ParameterStruct params;
  params.m_UseCtRange=m_UseCtRange;
  params.m_Range = m_Range;
  params.m_Direction = GetDirection();

  AccessByItk_3(image, CalculateGrayLevelSizeZoneFeatures, mask, featureList,params);

  return featureList;
}

mitk::GIFGrayLevelSizeZone::FeatureNameListType mitk::GIFGrayLevelSizeZone::GetFeatureNames()
{
  FeatureNameListType featureList;
  featureList.push_back("SizeZone SmallZoneEmphasis Means");
  featureList.push_back("SizeZone SmallZoneEmphasis Std.");
  featureList.push_back("SizeZone LargeZoneEmphasis Means");
  featureList.push_back("SizeZone LargeZoneEmphasis Std.");
  featureList.push_back("SizeZone GreyLevelNonuniformity Means");
  featureList.push_back("SizeZone GreyLevelNonuniformity Std.");
  featureList.push_back("SizeZone SizeZoneNonuniformity Means");
  featureList.push_back("SizeZone SizeZoneNonuniformity Std.");
  featureList.push_back("SizeZone LowGreyLevelZoneEmphasis Means");
  featureList.push_back("SizeZone LowGreyLevelZoneEmphasis Std.");
  featureList.push_back("SizeZone HighGreyLevelZoneEmphasis Means");
  featureList.push_back("SizeZone HighGreyLevelZoneEmphasis Std.");
  featureList.push_back("SizeZone SmallZoneLowGreyLevelEmphasis Means");
  featureList.push_back("SizeZone SmallZoneLowGreyLevelEmphasis Std.");
  featureList.push_back("SizeZone SmallZoneHighGreyLevelEmphasis Means");
  featureList.push_back("SizeZone SmallZoneHighGreyLevelEmphasis Std.");
  featureList.push_back("SizeZone LargeZoneHighGreyLevelEmphasis Means");
  featureList.push_back("SizeZone LargeZoneHighGreyLevelEmphasis Std.");
  return featureList;
}



void mitk::GIFGrayLevelSizeZone::AddArguments(mitkCommandLineParser &parser)
{
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::String, "Use Co-occurence matrix", "calculates Co-occurence based features (new implementation)", us::Any());
  parser.addArgument(name + "::range", name + "::range", mitkCommandLineParser::String, "Cooc 2 Range", "Define the range that is used (Semicolon-separated)", us::Any());
  parser.addArgument(name + "::direction", name + "::dir", mitkCommandLineParser::String, "Int", "Allows to specify the direction for Cooc and RL. 0: All directions, 1: Only single direction (Test purpose), 2,3,4... without dimension 0,1,2... ", us::Any());
  parser.addArgument(name + "::ctrange", name + "::ctrange", mitkCommandLineParser::String, "Int", "Turn on if CT images are used", us::Any());
}

void
mitk::GIFGrayLevelSizeZone::CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &, const Image::Pointer &maskNoNAN, FeatureListType &featureList)
{
  auto parsedArgs = GetParameter();
  std::string name = GetOptionPrefix();

  if (parsedArgs.count(GetLongName()))
  {
    int direction = 0;
    if (parsedArgs.count(name + "::direction"))
    {
      direction = SplitDouble(parsedArgs[name + "::direction"].ToString(), ';')[0];
    }
    std::vector<double> ranges;
    if (parsedArgs.count(name + "::range"))
    {
      ranges = SplitDouble(parsedArgs[name + "::range"].ToString(), ';');
    }
    else
    {
      ranges.push_back(1);
    }

    if (parsedArgs.count(name + "::ctrange"))
    {
      SetUseCtRange(true);
    }


    for (std::size_t i = 0; i < ranges.size(); ++i)
    {
      MITK_INFO << "Start calculating coocurence with range " << ranges[i] << "....";
      this->SetRange(ranges[i]);
      this->SetDirection(direction);
      auto localResults = this->CalculateFeatures(feature, maskNoNAN);
      featureList.insert(featureList.end(), localResults.begin(), localResults.end());
      MITK_INFO << "Finished calculating coocurence with range " << ranges[i] << "....";
    }
  }

}


