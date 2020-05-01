/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

namespace
{
  struct GIFIntensityVolumeHistogramFeaturesParameters
  {
    mitk::IntensityQuantifier::Pointer quantifier;
    mitk::FeatureID id;
  };

  template<typename TPixel, unsigned int VImageDimension>
  void CalculateIntensityPeak(const itk::Image<TPixel, VImageDimension>* itkImage, const mitk::Image* mask, GIFIntensityVolumeHistogramFeaturesParameters params, mitk::GIFIntensityVolumeHistogramFeatures::FeatureListType& featureList)
  {
    typedef itk::Image<TPixel, VImageDimension> ImageType;
    typedef itk::Image<unsigned short, VImageDimension> MaskType;

    typename MaskType::Pointer itkMask = MaskType::New();
    mitk::CastToItkImage(mask, itkMask);

    mitk::IntensityQuantifier::Pointer quantifier = params.quantifier;

    itk::ImageRegionConstIterator<ImageType> iter(itkImage, itkImage->GetLargestPossibleRegion());
    itk::ImageRegionConstIterator<MaskType> iterMask(itkMask, itkMask->GetLargestPossibleRegion());

    MITK_INFO << "Quantification: " << quantifier->GetMinimum() << " to " << quantifier->GetMaximum() << " with " << quantifier->GetBins() << " bins";

    iter.GoToBegin();
    iterMask.GoToBegin();
    std::vector<double> hist;
    hist.resize(quantifier->GetBins(), 0);

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
    for (int i = quantifier->GetBins() - 1; i >= 0; --i)
    {
      hist[i] /= count;
      hist[i] += fraction;
      fraction = hist[i];
      if (!firstRound)
      {
        auc += 0.5 * (hist[i] + hist[i + 1]) / (quantifier->GetBins() - 1);
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

    featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Volume fraction at 0.10 intensity"), hist[index010]));
    featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Volume fraction at 0.90 intensity"), hist[index090]));
    featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Intensity at 0.10 volume"), intensity010));
    featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Intensity at 0.90 volume"), intensity090));
    featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Difference volume fraction at 0.10 and 0.90 intensity"), std::abs<double>(hist[index010] - hist[index090])));
    featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Difference intensity at 0.10 and 0.90 volume"), std::abs<double>(intensity090 - intensity010)));
    featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Area under IVH curve"), auc));
    //featureList.push_back(std::make_pair("Local Intensity Global Intensity Peak", globalPeakValue));
  }
}

mitk::GIFIntensityVolumeHistogramFeatures::GIFIntensityVolumeHistogramFeatures()
{
  SetLongName("intensity-volume-histogram");
  SetShortName("ivoh");
  SetFeatureClassName("Intensity Volume Histogram");
}

void mitk::GIFIntensityVolumeHistogramFeatures::AddArguments(mitkCommandLineParser& parser) const
{
  AddQuantifierArguments(parser);
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::Bool, "Use Local Intensity", "calculates local intensity based features", us::Any());
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::GIFIntensityVolumeHistogramFeatures::DoCalculateFeatures(const Image* image, const Image* mask)
{
  FeatureListType featureList;

  InitializeQuantifier(image, mask, 1000);

  MITK_INFO << "Start calculating local intensity features ....";
  GIFIntensityVolumeHistogramFeaturesParameters params;
  params.quantifier = GetQuantifier();
  params.id = this->CreateTemplateFeatureID();
  AccessByItk_3(image, CalculateIntensityPeak, mask, params, featureList);
  MITK_INFO << "Finished calculating local intensity features....";

  return featureList;
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::GIFIntensityVolumeHistogramFeatures::CalculateFeatures(const Image* image, const Image* mask, const Image*)
{
  return Superclass::CalculateFeatures(image, mask);
}
