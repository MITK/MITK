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

#include <mitkGIFNeighbourhoodGreyToneDifferenceFeatures.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>
#include <mitkPixelTypeMultiplex.h>
#include <mitkImagePixelReadAccessor.h>

// ITK
#include <itkImageRegionIteratorWithIndex.h>
#include <itkNeighborhoodIterator.h>
// STL
#include <limits>

struct GIFNeighbourhoodGreyToneDifferenceParameter
{
  int Range = 1;
  mitk::IntensityQuantifier::Pointer quantifier;
  std::string prefix;
};

template<typename TPixel, unsigned int VImageDimension>
static void
CalculateIntensityPeak(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, GIFNeighbourhoodGreyToneDifferenceParameter params, mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::FeatureListType & featureList)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned short, VImageDimension> MaskType;

  typename MaskType::Pointer itkMask = MaskType::New();
  mitk::CastToItkImage(mask, itkMask);

  typename ImageType::SizeType regionSize;
  regionSize.Fill(params.Range);

  itk::NeighborhoodIterator<ImageType> iter(regionSize, itkImage, itkImage->GetLargestPossibleRegion());
  itk::NeighborhoodIterator<MaskType> iterMask(regionSize, itkMask, itkMask->GetLargestPossibleRegion());

  std::vector<double> pVector;
  std::vector<double> sVector;
  pVector.resize(params.quantifier->GetBins(), 0);
  sVector.resize(params.quantifier->GetBins(), 0);

  int count = 0;
  while (!iter.IsAtEnd())
  {
    if (iterMask.GetCenterPixel() > 0)
    {
      int localCount = 0;
      double localMean = 0;
      unsigned int localIndex = params.quantifier->IntensityToIndex(iter.GetCenterPixel());
      for (itk::SizeValueType i = 0; i < iter.Size(); ++i)
      {
        if (i == (iter.Size() / 2))
          continue;
        if (iterMask.GetPixel(i) > 0)
        {
          ++localCount;
          localMean += params.quantifier->IntensityToIndex(iter.GetPixel(i)) + 1;
        }
      }
      if (localCount > 0)
      {
        localMean /= localCount;
      }
        localMean = std::abs<double>(localIndex + 1 - localMean);

        pVector[localIndex] += 1;
        sVector[localIndex] += localMean;
        ++count;


    }
    ++iterMask;
    ++iter;
  }

  unsigned int Ngp = 0;
  for (unsigned int i = 0; i < params.quantifier->GetBins(); ++i)
  {
    if (pVector[i] > 0.1)
    {
      ++Ngp;
    }
    pVector[i] /= count;
  }

  double sumS = 0;
  double sumStimesP = 0;

  double contrastA = 0;
  double busynessA = 0;
  double complexity = 0;
  double strengthA = 0;
  for (unsigned int i = 0; i < params.quantifier->GetBins(); ++i)
  {
    sumS += sVector[i];
    sumStimesP += pVector[i] * sVector[i];
    for (unsigned int j = 0; j < params.quantifier->GetBins(); ++j)
    {
      double iMinusj = 1.0*i - 1.0*j;
      contrastA += pVector[i] * pVector[j] * iMinusj*iMinusj;
      if ((pVector[i] > 0) && (pVector[j] > 0))
      {
        busynessA += std::abs<double>((i + 1.0)*pVector[i] - (j + 1.0)*pVector[j]);
        complexity += std::abs<double>(iMinusj)*(pVector[i] * sVector[i] + pVector[j] * sVector[j]) / (pVector[i] + pVector[j]);
        strengthA += (pVector[i] + pVector[j])*iMinusj*iMinusj;
      }
    }
  }
  double coarsness = 1.0 / std::min<double>(sumStimesP, 1000000);
  double contrast = 0;
  double busyness = 0;
  if (Ngp > 1)
  {
    contrast = contrastA / Ngp / (Ngp - 1) / count * sumS;
    busyness = sumStimesP / busynessA;
  }
  complexity /= count;
  double strength = 0;
  if (sumS > 0)
  {
    strength = strengthA / sumS;
  }

  std::string prefix = params.prefix;
  featureList.push_back(std::make_pair(prefix + "Coarsness", coarsness));
  featureList.push_back(std::make_pair(prefix + "Contrast", contrast));
  featureList.push_back(std::make_pair(prefix + "Busyness", busyness));
  featureList.push_back(std::make_pair(prefix + "Complexity", complexity));
  featureList.push_back(std::make_pair(prefix + "Strength", strength));
}


mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::GIFNeighbourhoodGreyToneDifferenceFeatures() :
m_Range(1)
{
  SetLongName("neighbourhood-grey-tone-difference");
  SetShortName("ngtd");
  SetFeatureClassName("Neighbourhood Grey Tone Difference");
}

mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::FeatureListType mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;

  InitializeQuantifierFromParameters(image, mask);

  GIFNeighbourhoodGreyToneDifferenceParameter params;
  params.Range = GetRange();
  params.quantifier = GetQuantifier();
  params.prefix = FeatureDescriptionPrefix();

  AccessByItk_3(image, CalculateIntensityPeak, mask, params, featureList);
  return featureList;
}

mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::FeatureNameListType mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::GetFeatureNames()
{
  FeatureNameListType featureList;
  return featureList;
}


void mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::AddArguments(mitkCommandLineParser &parser)
{
  AddQuantifierArguments(parser);
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::Bool, "Use Neighbourhood Grey Tone Difference", "calculates Neighborhood Grey Tone based features", us::Any());
  parser.addArgument(name + "::range", name + "::range", mitkCommandLineParser::Int, "Range for the local intensity", "Give the range that should be used for the local intensity in mm", us::Any());

}

void
mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &, FeatureListType &featureList)
{
  InitializeQuantifierFromParameters(feature, mask);
  std::string name = GetOptionPrefix();

  auto parsedArgs = GetParameter();
  if (parsedArgs.count(GetLongName()))
  {
    MITK_INFO << "Start calculating Neighbourhood Grey Tone Difference features ....";
    if (parsedArgs.count(name + "::range"))
    {
      int range = us::any_cast<int>(parsedArgs[name + "::range"]);
      this->SetRange(range);
    }
    auto localResults = this->CalculateFeatures(feature, mask);
    featureList.insert(featureList.end(), localResults.begin(), localResults.end());
    MITK_INFO << "Finished calculating Neighbourhood Grey Tone Difference features....";
  }
}

std::string mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::GetCurrentFeatureEncoding()
{
  std::ostringstream  ss;
  ss << m_Range;
  std::string strRange = ss.str();
  return QuantifierParameterString() + "_Range-" + ss.str();
}



