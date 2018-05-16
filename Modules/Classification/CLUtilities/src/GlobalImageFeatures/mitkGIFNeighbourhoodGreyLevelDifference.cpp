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

template<typename TPixel, unsigned int VImageDimension>
void
  CalculateGrayLevelNeighbourhoodGreyLevelDifferenceFeatures(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, mitk::GIFNeighbourhoodGreyLevelDifference::FeatureListType & featureList, mitk::GIFNeighbourhoodGreyLevelDifference::ParameterStruct params)
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
    case TextureFilterType::Coarseness :
      featureList.push_back(std::make_pair("NeighbourhoodGreyLevelDifference ("+ strRange+") Coarseness Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::Contrast :
      featureList.push_back(std::make_pair("NeighbourhoodGreyLevelDifference ("+ strRange+") Contrast Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::Busyness :
      featureList.push_back(std::make_pair("NeighbourhoodGreyLevelDifference ("+ strRange+") Busyness Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::Complexity :
      featureList.push_back(std::make_pair("NeighbourhoodGreyLevelDifference ("+ strRange+") Complexity Means",featureMeans->ElementAt(i)));
      break;
    case TextureFilterType::Strength :
      featureList.push_back(std::make_pair("NeighbourhoodGreyLevelDifference ("+ strRange+") Strength Means",featureMeans->ElementAt(i)));
      break;
    default:
      break;
    }
  }
}

mitk::GIFNeighbourhoodGreyLevelDifference::GIFNeighbourhoodGreyLevelDifference():
m_Range(1.0), m_UseCtRange(false)
{
  SetShortName("ngld");
  SetLongName("NeighbourhoodGreyLevelDifference");
}

mitk::GIFNeighbourhoodGreyLevelDifference::FeatureListType mitk::GIFNeighbourhoodGreyLevelDifference::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;

  ParameterStruct params;
  params.m_UseCtRange=m_UseCtRange;
  params.m_Range = m_Range;
  params.m_Direction = GetDirection();

  AccessByItk_3(image, CalculateGrayLevelNeighbourhoodGreyLevelDifferenceFeatures, mask, featureList,params);

  return featureList;
}

mitk::GIFNeighbourhoodGreyLevelDifference::FeatureNameListType mitk::GIFNeighbourhoodGreyLevelDifference::GetFeatureNames()
{
  FeatureNameListType featureList;
  featureList.push_back("NeighbourhoodGreyLevelDifference. Coarseness Means");
  featureList.push_back("NeighbourhoodGreyLevelDifference. Coarseness Std.");
  featureList.push_back("NeighbourhoodGreyLevelDifference. Contrast Means");
  featureList.push_back("NeighbourhoodGreyLevelDifference. Contrast Std.");
  featureList.push_back("NeighbourhoodGreyLevelDifference. Busyness Means");
  featureList.push_back("NeighbourhoodGreyLevelDifference. Busyness Std.");
  featureList.push_back("NeighbourhoodGreyLevelDifference. Complexity Means");
  featureList.push_back("NeighbourhoodGreyLevelDifference. Complexity Std.");
  featureList.push_back("NeighbourhoodGreyLevelDifference. Strength Means");
  featureList.push_back("NeighbourhoodGreyLevelDifference. Strength Std.");
  return featureList;
}



void mitk::GIFNeighbourhoodGreyLevelDifference::AddArguments(mitkCommandLineParser &parser)
{
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::String, "Use Co-occurence matrix", "calculates Co-occurence based features (new implementation)", us::Any());
  parser.addArgument(name + "::range", name + "::range", mitkCommandLineParser::String, "Cooc 2 Range", "Define the range that is used (Semicolon-separated)", us::Any());
  parser.addArgument(name + "::direction", name + "::dir", mitkCommandLineParser::String, "Int", "Allows to specify the direction for Cooc and RL. 0: All directions, 1: Only single direction (Test purpose), 2,3,4... without dimension 0,1,2... ", us::Any());
}

void
mitk::GIFNeighbourhoodGreyLevelDifference::CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &, const Image::Pointer &maskNoNAN, FeatureListType &featureList)
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
      MITK_INFO << "Start calculating Neighbourhood Grey Level Difference with range " << ranges[i] << "....";
      this->SetRange(ranges[i]);
      this->SetDirection(direction);
      auto localResults = this->CalculateFeatures(feature, maskNoNAN);
      featureList.insert(featureList.end(), localResults.begin(), localResults.end());
      MITK_INFO << "Finished calculating coocurence with range " << ranges[i] << "....";
    }
  }

}

