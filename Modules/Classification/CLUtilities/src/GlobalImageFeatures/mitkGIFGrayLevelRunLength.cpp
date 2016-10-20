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
  requestedFeatures->push_back(TextureFilterType::GreyLevelNonuniformityNormalized);
  requestedFeatures->push_back(TextureFilterType::RunLengthNonuniformity);
  requestedFeatures->push_back(TextureFilterType::RunLengthNonuniformityNormalized);
  requestedFeatures->push_back(TextureFilterType::LowGreyLevelRunEmphasis);
  requestedFeatures->push_back(TextureFilterType::HighGreyLevelRunEmphasis);
  requestedFeatures->push_back(TextureFilterType::ShortRunLowGreyLevelEmphasis);
  requestedFeatures->push_back(TextureFilterType::ShortRunHighGreyLevelEmphasis);
  requestedFeatures->push_back(TextureFilterType::LongRunLowGreyLevelEmphasis);
  requestedFeatures->push_back(TextureFilterType::LongRunHighGreyLevelEmphasis);
  requestedFeatures->push_back(TextureFilterType::RunPercentage);
  requestedFeatures->push_back(TextureFilterType::NumberOfRuns);
  requestedFeatures->push_back(TextureFilterType::GreyLevelVariance);
  requestedFeatures->push_back(TextureFilterType::RunLengthVariance);
  requestedFeatures->push_back(TextureFilterType::RunEntropy);

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
    MITK_INFO << "Min: " << minMaxComputer->GetMinimum() << " Max: " << minMaxComputer->GetMaximum() << " Range: " << rangeOfPixels;
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
    case TextureFilterType::GreyLevelNonuniformityNormalized :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") GreyLevelNonuniformityNormalized Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") GreyLevelNonuniformityNormalized Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::RunLengthNonuniformity :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") RunLengthNonuniformity Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") RunLengthNonuniformity Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::RunLengthNonuniformityNormalized :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") RunLengthNonuniformityNormalized Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") RunLengthNonuniformityNormalized Std.",featureStd->ElementAt(i)));
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
    case TextureFilterType::GreyLevelVariance :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") GreyLevelVariance Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") GreyLevelVariance Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::RunLengthVariance :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") RunLengthVariance Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") RunLengthVariance Std.",featureStd->ElementAt(i)));
      break;
    case TextureFilterType::RunEntropy :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") RunEntropy Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") RunEntropy Std.",featureStd->ElementAt(i)));
      break;
    default:
      break;
    }
  }
}

mitk::GIFGrayLevelRunLength::GIFGrayLevelRunLength():
m_Range(1.0), m_UseCtRange(false), m_Direction(0)
{
  SetShortName("rl");
  SetLongName("run-length");
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


void mitk::GIFGrayLevelRunLength::AddArguments(mitkCommandLineParser &parser)
{
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::String, "Use Co-occurence matrix", "calculates Co-occurence based features (new implementation)", us::Any());
  parser.addArgument(name + "::range", name + "::range", mitkCommandLineParser::String, "Cooc 2 Range", "Define the range that is used (Semicolon-separated)", us::Any());
  parser.addArgument(name + "::direction", name + "::dir", mitkCommandLineParser::String, "Int", "Allows to specify the direction for Cooc and RL. 0: All directions, 1: Only single direction (Test purpose), 2,3,4... without dimension 0,1,2... ", us::Any());
}

void
mitk::GIFGrayLevelRunLength::CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList)
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

    for (std::size_t i = 0; i < ranges.size(); ++i)
    {
      MITK_INFO << "Start calculating Run-length with range " << ranges[i] << "....";
      this->SetRange(ranges[i]);
      this->SetDirection(direction);
      auto localResults = this->CalculateFeatures(feature, maskNoNAN);
      featureList.insert(featureList.end(), localResults.begin(), localResults.end());
      MITK_INFO << "Finished calculating Run-length with range " << ranges[i] << "....";
    }
  }

}
