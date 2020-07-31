/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkGIFNeighbourhoodGreyLevelDifference.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>

// ITK
#include <itkEnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter.h>
#include <itkMinimumMaximumImageCalculator.h>

// STL
#include <sstream>

struct GIFNeighbourhoodGreyLevelDifferenceParameterStruct
{
  bool  m_UseCTRange;
  double m_Range;
  unsigned int m_Direction;
  mitk::FeatureID id;
};

template<typename TPixel, unsigned int VImageDimension>
void
  CalculateGrayLevelNeighbourhoodGreyLevelDifferenceFeatures(const itk::Image<TPixel, VImageDimension>* itkImage, const mitk::Image* mask, mitk::GIFNeighbourhoodGreyLevelDifference::FeatureListType & featureList, GIFNeighbourhoodGreyLevelDifferenceParameterStruct params)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<TPixel, VImageDimension> MaskType;
  typedef itk::Statistics::EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter<ImageType> FilterType;
  typedef itk::MinimumMaximumImageCalculator<ImageType> MinMaxComputerType;
  typedef typename FilterType::NeighbourhoodGreyLevelDifferenceFeaturesFilterType TextureFilterType;

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
  requestedFeatures->push_back(TextureFilterType::Coarseness);
  requestedFeatures->push_back(TextureFilterType::Contrast);
  requestedFeatures->push_back(TextureFilterType::Busyness);
  requestedFeatures->push_back(TextureFilterType::Complexity);
  requestedFeatures->push_back(TextureFilterType::Strength);

  typename MinMaxComputerType::Pointer minMaxComputer = MinMaxComputerType::New();
  minMaxComputer->SetImage(itkImage);
  minMaxComputer->Compute();

  filter->SetInput(itkImage);
  filter->SetMaskImage(maskImage);
  filter->SetRequestedFeatures(requestedFeatures);
  int rangeOfPixels = params.m_Range;
  if (rangeOfPixels < 2)
    rangeOfPixels = 256;

  if (params.m_UseCTRange)
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
    case TextureFilterType::Coarseness :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Coarseness Means"),featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::Contrast :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Contrast Means"),featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::Busyness :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Busyness Means"),featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::Complexity :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Complexity Means"),featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::Strength :
      featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Strength Means"),featureMeans->ElementAt(i)));
      break;
    default:
      break;
    }
  }
}

mitk::GIFNeighbourhoodGreyLevelDifference::GIFNeighbourhoodGreyLevelDifference():
  m_Ranges({ 1.0 }), m_UseCTRange(false)
{
  SetShortName("ngld");
  SetLongName("NeighbourhoodGreyLevelDifference");
}

void mitk::GIFNeighbourhoodGreyLevelDifference::SetRanges(std::vector<double> ranges)
{
  m_Ranges = ranges;
  this->Modified();
}

void mitk::GIFNeighbourhoodGreyLevelDifference::SetRange(double range)
{
  m_Ranges.resize(1);
  m_Ranges[0] = range;
  this->Modified();
}

void mitk::GIFNeighbourhoodGreyLevelDifference::AddArguments(mitkCommandLineParser &parser) const
{
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::String, "Use Co-occurence matrix", "calculates Co-occurence based features (new implementation)", us::Any());
  parser.addArgument(name + "::range", name + "::range", mitkCommandLineParser::String, "Cooc 2 Range", "Define the range that is used (Semicolon-separated)", us::Any());
  parser.addArgument(name + "::direction", name + "::dir", mitkCommandLineParser::Int, "Int", "Allows to specify the direction for Cooc and RL. 0: All directions, 1: Only single direction (Test purpose), 2,3,4... without dimension 0,1,2... ", us::Any());
  parser.addArgument(name + "::useCTRange", name + "::ct", mitkCommandLineParser::Bool, "Use CT range", "If flag is set only value in the CT range will be used for feature computation.", us::Any());
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::GIFNeighbourhoodGreyLevelDifference::DoCalculateFeatures(const Image* image, const Image* mask)
{
  FeatureListType featureList;

  for (const auto& range : m_Ranges)
  {
    MITK_INFO << "Start calculating Neighbourhood Grey Level Difference with range " << range << "....";
    GIFNeighbourhoodGreyLevelDifferenceParameterStruct params;
    params.m_UseCTRange = m_UseCTRange;
    params.m_Range = range;
    params.m_Direction = GetDirection();
    params.id = this->CreateTemplateFeatureID(std::to_string(range), { {GetOptionPrefix() + "::range", range} });
    AccessByItk_3(image, CalculateGrayLevelNeighbourhoodGreyLevelDifferenceFeatures, mask, featureList, params);
    MITK_INFO << "Finished calculating coocurence with range " << range << "....";
  }

  return featureList;
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::GIFNeighbourhoodGreyLevelDifference::CalculateFeatures(const Image* image, const Image*, const Image* maskNoNAN)
{
  return Superclass::CalculateFeatures(image, maskNoNAN);
}

std::string mitk::GIFNeighbourhoodGreyLevelDifference::GenerateLegacyFeatureName(const FeatureID& id) const
{
  return "NeighbourhoodGreyLevelDifference (" + id.parameters.at(this->GetOptionPrefix() + "::range").ToString() + ") " + id.name;
}

void mitk::GIFNeighbourhoodGreyLevelDifference::ConfigureSettingsByParameters(const ParametersType& parameters)
{
  auto prefixname = GetOptionPrefix();

  auto name = prefixname + "::range";
  if (parameters.count(name))
  {
    m_Ranges = SplitDouble(parameters.at(name).ToString(), ';');
  }

  name = prefixname + "::direction";
  if (parameters.count(name))
  {
    int direction = us::any_cast<int>(parameters.at(name));
    this->SetDirection(direction);
  }

  name = prefixname + "::useCTRange";
  if (parameters.count(name))
  {
    bool tmp = us::any_cast<bool>(parameters.at(name));
    m_UseCTRange = tmp;
  }
}
