/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  mitk::FeatureID id;
};

template<typename TPixel, unsigned int VImageDimension>
static void
CalculateIntensityPeak(const itk::Image<TPixel, VImageDimension>* itkImage, const mitk::Image* mask, GIFNeighbourhoodGreyToneDifferenceParameter params, mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::FeatureListType & featureList)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned short, VImageDimension> MaskType;

  typename MaskType::Pointer itkMask = MaskType::New();
  mitk::CastToItkImage(mask, itkMask);

  typename ImageType::SizeType regionSize;
  regionSize.Fill(params.Range);

  itk::ConstNeighborhoodIterator<ImageType> iter(regionSize, itkImage, itkImage->GetLargestPossibleRegion());
  itk::ConstNeighborhoodIterator<MaskType> iterMask(regionSize, itkMask, itkMask->GetLargestPossibleRegion());

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

  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Coarsness"), coarsness));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Contrast"), contrast));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Busyness"), busyness));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Complexity"), complexity));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Strength"), strength));
}


mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::GIFNeighbourhoodGreyToneDifferenceFeatures() :
m_Range(1)
{
  SetLongName("neighbourhood-grey-tone-difference");
  SetShortName("ngtd");
  SetFeatureClassName("Neighbourhood Grey Tone Difference");
}

std::string mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::GenerateLegacyFeatureEncoding(const FeatureID& id) const
{
  return this->QuantifierParameterString()+"_Range-" + id.parameters.at(this->GetOptionPrefix() + "::range").ToString();
}

void mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::AddArguments(mitkCommandLineParser& parser) const
{
  AddQuantifierArguments(parser);
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::Bool, "Use Neighbourhood Grey Tone Difference", "calculates Neighborhood Grey Tone based features", us::Any());
  parser.addArgument(name + "::range", name + "::range", mitkCommandLineParser::Int, "Range for the local intensity", "Give the range that should be used for the local intensity in mm", us::Any());
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::DoCalculateFeatures(const Image* image, const Image* mask)
{
  FeatureListType featureList;

  InitializeQuantifier(image, mask);

  MITK_INFO << "Start calculating Neighbourhood Grey Tone Difference features ....";

  GIFNeighbourhoodGreyToneDifferenceParameter params;
  params.Range = GetRange();
  params.quantifier = GetQuantifier();
  params.id = this->CreateTemplateFeatureID();

  AccessByItk_3(image, CalculateIntensityPeak, mask, params, featureList);

  MITK_INFO << "Finished calculating Neighbourhood Grey Tone Difference features....";

  return featureList;
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::CalculateFeatures(const Image* image, const Image* mask, const Image*)
{
  return Superclass::CalculateFeatures(image, mask);
}

void mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::ConfigureSettingsByParameters(const ParametersType& parameters)
{
  auto name = GetOptionPrefix() + "::range";
  if (parameters.count(name))
  {
    int range = us::any_cast<int>(parameters.at(name));
    this->SetRange(range);
  }
}
