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

#include <mitkGIFIntensityVolumeHistogramFeatures.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>
#include <mitkPixelTypeMultiplex.h>

// ITK
#include <itkImageRegionIteratorWithIndex.h>
#include <itkNeighborhoodIterator.h>
// STL
#include <limits>

struct GIFIntensityVolumeHistogramFeaturesParameters
{
  mitk::IntensityQuantifier::Pointer quantifier;
  std::string prefix;
};


template<typename TPixel, unsigned int VImageDimension>
static void
CalculateIntensityPeak(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, GIFIntensityVolumeHistogramFeaturesParameters params, mitk::GIFIntensityVolumeHistogramFeatures::FeatureListType & featureList)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned short, VImageDimension> MaskType;

  typename MaskType::Pointer itkMask = MaskType::New();
  mitk::CastToItkImage(mask, itkMask);

  mitk::IntensityQuantifier::Pointer quantifier = params.quantifier;
  std::string prefix = params.prefix;

  itk::ImageRegionConstIterator<ImageType> iter(itkImage, itkImage->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<MaskType> iterMask(itkMask, itkMask->GetLargestPossibleRegion());

  MITK_INFO << "Quantification: " << quantifier->GetMinimum() << " to " << quantifier->GetMaximum() << " with " << quantifier->GetBins()<< " bins";

  iter.GoToBegin();
  iterMask.GoToBegin();
  std::vector<double> hist;
  hist.resize(quantifier->GetBins() , 0);

  int count = 0;
  while (!iter.IsAtEnd())
  {
    if (iterMask.Get() > 0)
    {
      double value = iter.Get();
      //std::size_t index = std::floor((value - minimum) / (maximum - minimum) * (bins-1));
      std::size_t index = quantifier->IntensityToIndex(value);
      ++count;
      hist[index] += 1.0;// / count;
    }
    ++iterMask;
    ++iter;
  }

  bool notFoundIntenstiy010 = true;
  bool notFoundIntenstiy090 = true;

  double intensity010 = -1;
  double intensity090 = -1;
  double fraction = 0;
  double auc = 0;
  bool firstRound = true;
  for (int i = quantifier->GetBins()-1; i >= 0; --i)
  {
    hist[i] /= count;
    hist[i] += fraction;
    fraction = hist[i];
    if (!firstRound)
    {
      auc += 0.5 * (hist[i] + hist[i+1]) / (quantifier->GetBins()-1);
    }
    firstRound = false;

    if (notFoundIntenstiy010 && fraction > 0.1)
    {
      intensity010 = quantifier->IndexToMeanIntensity(i + 1);
      notFoundIntenstiy010 = false;
    }
    if (notFoundIntenstiy090 && fraction > 0.9)
    {
      intensity090 = quantifier->IndexToMeanIntensity(i + 1);
      notFoundIntenstiy090 = false;
    }
  }

  unsigned int index010 = std::ceil(quantifier->GetBins() * 0.1);
  unsigned int index090 = std::floor(quantifier->GetBins() * 0.9);

  featureList.push_back(std::make_pair(prefix + "Volume fraction at 0.10 intensity", hist[index010]));
  featureList.push_back(std::make_pair(prefix + "Volume fraction at 0.90 intensity", hist[index090]));
  featureList.push_back(std::make_pair(prefix + "Intensity at 0.10 volume", intensity010));
  featureList.push_back(std::make_pair(prefix + "Intensity at 0.90 volume", intensity090));
  featureList.push_back(std::make_pair(prefix + "Difference volume fraction at 0.10 and 0.90 intensity", std::abs<double>(hist[index010] - hist[index090])));
  featureList.push_back(std::make_pair(prefix + "Difference intensity at 0.10 and 0.90 volume", std::abs<double>(intensity090 - intensity010)));
  featureList.push_back(std::make_pair(prefix + "Area under IVH curve", auc));
  //featureList.push_back(std::make_pair("Local Intensity Global Intensity Peak", globalPeakValue));
}


mitk::GIFIntensityVolumeHistogramFeatures::GIFIntensityVolumeHistogramFeatures()
{
  SetLongName("intensity-volume-histogram");
  SetShortName("ivoh");
  SetFeatureClassName("Intensity Volume Histogram");
}

mitk::GIFIntensityVolumeHistogramFeatures::FeatureListType mitk::GIFIntensityVolumeHistogramFeatures::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  InitializeQuantifier(image, mask, 1000);
  FeatureListType featureList;
  GIFIntensityVolumeHistogramFeaturesParameters params;
  params.quantifier = GetQuantifier();
  params.prefix = FeatureDescriptionPrefix();
  AccessByItk_3(image, CalculateIntensityPeak, mask, params, featureList);
  return featureList;
}

mitk::GIFIntensityVolumeHistogramFeatures::FeatureNameListType mitk::GIFIntensityVolumeHistogramFeatures::GetFeatureNames()
{
  FeatureNameListType featureList;
  return featureList;
}


void mitk::GIFIntensityVolumeHistogramFeatures::AddArguments(mitkCommandLineParser &parser)
{
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::Bool, "Use Local Intensity", "calculates local intensity based features", us::Any());
  AddQuantifierArguments(parser);
}

void
mitk::GIFIntensityVolumeHistogramFeatures::CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &, FeatureListType &featureList)
{
  InitializeQuantifierFromParameters(feature, mask, 1000);

  auto parsedArgs = GetParameter();
  if (parsedArgs.count(GetLongName()))
  {
    MITK_INFO << "Start calculating local intensity features ....";
    auto localResults = this->CalculateFeatures(feature, mask);
    featureList.insert(featureList.end(), localResults.begin(), localResults.end());
    MITK_INFO << "Finished calculating local intensity features....";
  }
}

std::string mitk::GIFIntensityVolumeHistogramFeatures::GetCurrentFeatureEncoding()
{
  return QuantifierParameterString();
}

