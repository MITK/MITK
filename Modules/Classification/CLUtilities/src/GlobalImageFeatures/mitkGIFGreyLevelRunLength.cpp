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

#include <mitkGIFGreyLevelRunLength.h>

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
  CalculateGrayLevelRunLengthFeatures(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, mitk::GIFGreyLevelRunLength::FeatureListType & featureList, mitk::GIFGreyLevelRunLength::ParameterStruct params)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<TPixel, VImageDimension> MaskType;
  typedef itk::Statistics::EnhancedScalarImageToRunLengthFeaturesFilter<ImageType> FilterType;
  typedef itk::MinimumMaximumImageCalculator<ImageType> MinMaxComputerType;
  typedef typename FilterType::RunLengthFeaturesFilterType TextureFilterType;

  typename MaskType::Pointer maskImage = MaskType::New();
  mitk::CastToItkImage(mask, maskImage);

  typename FilterType::Pointer filter = FilterType::New();
  typename FilterType::Pointer filter2 = FilterType::New();

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
  filter2->SetOffsets(newOffset);


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
  filter2->SetInput(itkImage);
  filter2->SetMaskImage(maskImage);
  filter2->SetRequestedFeatures(requestedFeatures);
  int rangeOfPixels = params.Bins;
  if (rangeOfPixels < 2)
    rangeOfPixels = 256;

  if (params.m_UseCtRange)
  {
    filter->SetPixelValueMinMax((TPixel)(-1024.5),(TPixel)(3096.5));
    filter->SetNumberOfBinsPerAxis(3096.5 + 1024.5);
    filter2->SetPixelValueMinMax((TPixel)(-1024.5), (TPixel)(3096.5));
    filter2->SetNumberOfBinsPerAxis(3096.5 + 1024.5);
  } else
  {
    double minRange = minMaxComputer->GetMinimum() - 0.5;
    double maxRange = minMaxComputer->GetMaximum() + 0.5;


    if (params.UseMinimumIntensity)
      minRange = params.MinimumIntensity;
    if (params.UseMaximumIntensity)
      maxRange = params.MaximumIntensity;

    filter->SetPixelValueMinMax(minRange, maxRange);
    filter->SetNumberOfBinsPerAxis(rangeOfPixels);
    filter2->SetPixelValueMinMax(minRange, maxRange);
    filter2->SetNumberOfBinsPerAxis(rangeOfPixels);
  }

  filter->SetDistanceValueMinMax(0, rangeOfPixels);
  filter2->SetDistanceValueMinMax(0, rangeOfPixels);

  filter2->CombinedFeatureCalculationOn();

  filter->Update();
  filter2->Update();

  auto featureMeans = filter->GetFeatureMeans ();
  auto featureStd = filter->GetFeatureStandardDeviations();
  auto featureCombined = filter2->GetFeatureMeans();

  std::ostringstream  ss;
  ss << rangeOfPixels;
  std::string strRange = ss.str();
  for (std::size_t i = 0; i < featureMeans->size(); ++i)
  {
    switch (i)
    {
    case TextureFilterType::ShortRunEmphasis :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") ShortRunEmphasis Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") ShortRunEmphasis Std.", featureStd->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") ShortRunEmphasis Comb.", featureCombined->ElementAt(i)));
      break;
    case TextureFilterType::LongRunEmphasis :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") LongRunEmphasis Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") LongRunEmphasis Std.", featureStd->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") LongRunEmphasis Comb.", featureCombined->ElementAt(i)));
      break;
    case TextureFilterType::GreyLevelNonuniformity :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") GreyLevelNonuniformity Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") GreyLevelNonuniformity Std.", featureStd->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") GreyLevelNonuniformity Comb.", featureCombined->ElementAt(i)));
      break;
    case TextureFilterType::GreyLevelNonuniformityNormalized :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") GreyLevelNonuniformityNormalized Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") GreyLevelNonuniformityNormalized Std.", featureStd->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") GreyLevelNonuniformityNormalized Comb.", featureCombined->ElementAt(i)));
      break;
    case TextureFilterType::RunLengthNonuniformity :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") RunLengthNonuniformity Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") RunLengthNonuniformity Std.", featureStd->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") RunLengthNonuniformity Comb.", featureCombined->ElementAt(i)));
      break;
    case TextureFilterType::RunLengthNonuniformityNormalized :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") RunLengthNonuniformityNormalized Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") RunLengthNonuniformityNormalized Std.", featureStd->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") RunLengthNonuniformityNormalized Comb.", featureCombined->ElementAt(i)));
      break;
    case TextureFilterType::LowGreyLevelRunEmphasis :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") LowGreyLevelRunEmphasis Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") LowGreyLevelRunEmphasis Std.", featureStd->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") LowGreyLevelRunEmphasis Comb.", featureCombined->ElementAt(i)));
      break;
    case TextureFilterType::HighGreyLevelRunEmphasis :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") HighGreyLevelRunEmphasis Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") HighGreyLevelRunEmphasis Std.", featureStd->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") HighGreyLevelRunEmphasis Comb.", featureCombined->ElementAt(i)));
      break;
    case TextureFilterType::ShortRunLowGreyLevelEmphasis :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") ShortRunLowGreyLevelEmphasis Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") ShortRunLowGreyLevelEmphasis Std.", featureStd->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") ShortRunLowGreyLevelEmphasis Comb.", featureCombined->ElementAt(i)));
      break;
    case TextureFilterType::ShortRunHighGreyLevelEmphasis :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") ShortRunHighGreyLevelEmphasis Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") ShortRunHighGreyLevelEmphasis Std.", featureStd->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") ShortRunHighGreyLevelEmphasis Comb.", featureCombined->ElementAt(i)));
      break;
    case TextureFilterType::LongRunLowGreyLevelEmphasis :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") LongRunLowGreyLevelEmphasis Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") LongRunLowGreyLevelEmphasis Std.", featureStd->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") LongRunLowGreyLevelEmphasis Comb.", featureCombined->ElementAt(i)));
      break;
    case TextureFilterType::LongRunHighGreyLevelEmphasis :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") LongRunHighGreyLevelEmphasis Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") LongRunHighGreyLevelEmphasis Std.", featureStd->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") LongRunHighGreyLevelEmphasis Comb.", featureCombined->ElementAt(i)));
      break;
    case TextureFilterType::RunPercentage :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") RunPercentage Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") RunPercentage Std.", featureStd->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") RunPercentage Comb.", featureCombined->ElementAt(i)/newOffset->size()));
      break;
    case TextureFilterType::NumberOfRuns :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") NumberOfRuns Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") NumberOfRuns Std.", featureStd->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") NumberOfRuns Comb.", featureCombined->ElementAt(i)));
      break;
    case TextureFilterType::GreyLevelVariance :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") GreyLevelVariance Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") GreyLevelVariance Std.", featureStd->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") GreyLevelVariance Comb.", featureCombined->ElementAt(i)));
      break;
    case TextureFilterType::RunLengthVariance :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") RunLengthVariance Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") RunLengthVariance Std.", featureStd->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") RunLengthVariance Comb.", featureCombined->ElementAt(i)));
      break;
    case TextureFilterType::RunEntropy :
      featureList.push_back(std::make_pair("RunLength. ("+ strRange+") RunEntropy Means",featureMeans->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") RunEntropy Std.", featureStd->ElementAt(i)));
      featureList.push_back(std::make_pair("RunLength. (" + strRange + ") RunEntropy Comb.", featureCombined->ElementAt(i)));
      break;
    default:
      break;
    }
  }
}

mitk::GIFGreyLevelRunLength::GIFGreyLevelRunLength():
m_Range(1.0), m_UseCtRange(false)
{
  SetShortName("rl");
  SetLongName("run-length");
}

mitk::GIFGreyLevelRunLength::FeatureListType mitk::GIFGreyLevelRunLength::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;

  ParameterStruct params;
  params.m_UseCtRange=m_UseCtRange;
  params.m_Range = m_Range;
  params.m_Direction = GetDirection();

  params.MinimumIntensity = GetMinimumIntensity();
  params.MaximumIntensity = GetMaximumIntensity();
  params.UseMinimumIntensity = GetUseMinimumIntensity();
  params.UseMaximumIntensity = GetUseMaximumIntensity();
  params.Bins = GetBins();

  MITK_INFO << params.MinimumIntensity;
  MITK_INFO << params.MaximumIntensity;
  MITK_INFO << params.m_UseCtRange;
  MITK_INFO << params.m_Direction;
  MITK_INFO << params.Bins;
  MITK_INFO << params.m_Range;

  AccessByItk_3(image, CalculateGrayLevelRunLengthFeatures, mask, featureList,params);

  return featureList;
}

mitk::GIFGreyLevelRunLength::FeatureNameListType mitk::GIFGreyLevelRunLength::GetFeatureNames()
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


void mitk::GIFGreyLevelRunLength::AddArguments(mitkCommandLineParser &parser)
{
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::String, "Use Co-occurence matrix", "calculates Co-occurence based features (new implementation)", us::Any());
  parser.addArgument(name + "::bins", name + "::bins", mitkCommandLineParser::String, "RL Bins", "Define the number of bins that is used (Semicolon-separated)", us::Any());
}

void
mitk::GIFGreyLevelRunLength::CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &, const Image::Pointer &maskNoNAN, FeatureListType &featureList)
{
  auto parsedArgs = GetParameter();
  std::string name = GetOptionPrefix();

  if (parsedArgs.count(GetLongName()))
  {
    std::vector<double> bins;
    bins.push_back(GetBins());
    if (parsedArgs.count(name + "::bins"))
    {
      bins = SplitDouble(parsedArgs[name + "::bins"].ToString(), ';');
    }

    for (std::size_t i = 0; i < bins.size(); ++i)
    {
      MITK_INFO << "Start calculating Run-length with range " << bins[i] << "....";
      this->SetBins(bins[i]);
      auto localResults = this->CalculateFeatures(feature, maskNoNAN);
      featureList.insert(featureList.end(), localResults.begin(), localResults.end());
      MITK_INFO << "Finished calculating Run-length with range " << bins[i] << "....";
    }
  }

}
