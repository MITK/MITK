/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkGIFFirstOrderNumericStatistics.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>

// ITK
#include <itkLabelStatisticsImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkImageRegionIterator.h>

// STL
#include <sstream>

struct FirstOrderNumericParameterStruct {
  mitk::IntensityQuantifier::Pointer quantifier;
  double MinimumIntensity;
  double MaximumIntensity;
  int Bins;
  mitk::FeatureID id;
};

template<typename TPixel, unsigned int VImageDimension>
void
CalculateFirstOrderStatistics(const itk::Image<TPixel, VImageDimension>* itkImage, const mitk::Image* mask, mitk::GIFFirstOrderNumericStatistics::FeatureListType & featureList, FirstOrderNumericParameterStruct params)
{
  typedef itk::Image<TPixel, VImageDimension>             ImageType;
  typedef itk::Image<unsigned short, VImageDimension>     MaskType;

  typename MaskType::Pointer maskImage = MaskType::New();
  mitk::CastToItkImage(mask, maskImage);

  //
  // Calculate the Volume of Voxel (Maximum to the 3th order)
  //
  double voxelVolume = 1;
  for (unsigned int i = 0; i < std::min<unsigned int>(3, VImageDimension); ++i)
    voxelVolume *= itkImage->GetSpacing()[i];

  //
  // Calculate the Hypervolume of Voxel
  //
  double voxelSpace = 1;
  for (unsigned int i = 0; i < VImageDimension; ++i)
    voxelSpace *= itkImage->GetSpacing()[i];

  unsigned int numberOfBins = params.quantifier->GetBins();
  std::vector<unsigned int> histogram;
  histogram.resize(numberOfBins, 0);


  double minimum = std::numeric_limits<double>::max();
  double maximum = std::numeric_limits<double>::lowest();
  double absoluteMinimum = std::numeric_limits<double>::max();
  double absoluteMaximum = std::numeric_limits<double>::lowest();
  double sum = 0;
  double sumTwo= 0;
  double sumThree = 0;
  unsigned int numberOfVoxels = 0;

  itk::ImageRegionConstIterator<ImageType> imageIter(itkImage, itkImage->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<MaskType>  maskIter(maskImage, maskImage->GetLargestPossibleRegion());

  while (!imageIter.IsAtEnd())
  {
    double value = imageIter.Get();

    absoluteMinimum = std::min<double>(minimum, value);
    absoluteMaximum = std::max<double>(maximum, value);
    if (maskIter.Get() > 0)
    {
      minimum = std::min<double>(minimum, value);
      maximum = std::max<double>(maximum, value);

      sum += value;
      sumTwo += value * value;
      sumThree += value * value*value;

      histogram[params.quantifier->IntensityToIndex(value)] += 1;

      ++numberOfVoxels;
    }
    ++maskIter;
    ++imageIter;
  }

  //
  // Histogram based calculations
  //
  unsigned int passedValues = 0;
  double doubleNoOfVoxes = numberOfVoxels;
  double median = 0;
  double lastIntensityWithValues = params.quantifier->IndexToMeanIntensity(0);
  std::size_t modeIdx = 0;

  double entropy = 0;
  double uniformity = 0;

  std::vector<double> percentiles;
  percentiles.resize(20, 0);

  for (std::size_t idx = 0; idx < histogram.size(); ++idx)
  {
    unsigned int actualValues = histogram[idx];

    for (std::size_t percentileIdx = 0; percentileIdx < percentiles.size(); ++percentileIdx)
    {
      double threshold = doubleNoOfVoxes * (percentileIdx + 1) *1.0 / (percentiles.size());
      if ((passedValues < threshold) & ((passedValues + actualValues) >= threshold))
      {
        // Lower Bound
        if (passedValues == std::floor(threshold))
        {
          percentiles[percentileIdx] = 0.5*(lastIntensityWithValues + params.quantifier->IndexToMeanIntensity(idx));
        }
        else
        {
          percentiles[percentileIdx] = params.quantifier->IndexToMeanIntensity(idx);
        }
      }
    }

    if ((passedValues < doubleNoOfVoxes * 0.5) & ((passedValues + actualValues) >= doubleNoOfVoxes * 0.5))
    {
      // Lower Bound
      if (passedValues == std::floor(doubleNoOfVoxes * 0.5))
      {
        median = 0.5*(lastIntensityWithValues + params.quantifier->IndexToMeanIntensity(idx));
      }
      else
      {
        median = params.quantifier->IndexToMeanIntensity(idx);
      }
    }

    if (actualValues > histogram[modeIdx])
    {
      modeIdx = idx;
    }

    if (actualValues > 0)
    {
      lastIntensityWithValues = params.quantifier->IndexToMeanIntensity(idx);

      double currentProbability = actualValues / (1.0 *numberOfVoxels);
      uniformity += currentProbability * currentProbability;
      entropy += currentProbability * std::log(currentProbability) / std::log(2);
    }
    passedValues += actualValues;
  }
  double p10 = percentiles[1];
  //double p25idx = params.quantifier->IntensityToIndex(percentiles[4]);
  //double p75idx = params.quantifier->IntensityToIndex(percentiles[14]);
  double p25idx = percentiles[4];
  double p75idx = percentiles[14];
  double p90 = percentiles[17];

  double mean = sum / (numberOfVoxels);
  double variance = sumTwo / (numberOfVoxels) - (mean*mean);
  double energy = sumTwo;
  double rootMeanSquare = std::sqrt(sumTwo / numberOfVoxels);

  double sumAbsoluteDistanceToMean = 0;
  double sumAbsoluteDistanceToMedian = 0;
  double sumRobust = 0;
  double sumRobustSquare = 0;
  double sumRobustAbsolulteDistanceToMean = 0;
  double sumValueMinusMean = 0;
  double sumValueMinusMeanThree = 0;
  double sumValueMinusMeanFour = 0;
  unsigned int numberOfRobustVoxel = 0;


  maskIter.GoToBegin();
  imageIter.GoToBegin();
  while (!imageIter.IsAtEnd())
  {
    if (maskIter.Get() > 0)
    {
      double value = imageIter.Get();
      double valueMinusMean = value - mean;

      sumAbsoluteDistanceToMean += std::abs<double>(valueMinusMean);
      sumAbsoluteDistanceToMedian += std::abs<double>(value - median);
      sumValueMinusMean += valueMinusMean;
      sumValueMinusMeanThree += valueMinusMean * valueMinusMean * valueMinusMean;
      sumValueMinusMeanFour += valueMinusMean * valueMinusMean * valueMinusMean * valueMinusMean;

      if ((p10 <= value) & (value <= p90))
      {
        sumRobust += value;
        sumRobustSquare += value * value;
        ++numberOfRobustVoxel;
      }
    }
    ++maskIter;
    ++imageIter;
  }
  double robustMean = sumRobust / numberOfRobustVoxel;
  double robustVariance = sumRobustSquare / numberOfRobustVoxel - (robustMean * robustMean);

  maskIter.GoToBegin();
  imageIter.GoToBegin();
  while (!imageIter.IsAtEnd())
  {
    if (maskIter.Get() > 0)
    {
      double value = imageIter.Get();

      if ((p10 <= value) & (value <= p90))
      {
        sumRobustAbsolulteDistanceToMean += std::abs(value - robustMean);
      }
    }
    ++maskIter;
    ++imageIter;
  }



  double meanAbsoluteDeviation = sumAbsoluteDistanceToMean / numberOfVoxels;
  double medianAbsoluteDeviation = sumAbsoluteDistanceToMedian / numberOfVoxels;
  double robustMeanAbsoluteDeviation = sumRobustAbsolulteDistanceToMean / numberOfRobustVoxel;
  double skewness = sumValueMinusMeanThree / numberOfVoxels / variance / std::sqrt(variance);
  double kurtosis = sumValueMinusMeanFour / numberOfVoxels / variance / variance;
  double interquantileRange = p75idx - p25idx;
  double coefficientOfVariation = std::sqrt(variance) / mean;
  double quantileCoefficientOfDispersion = (p75idx - p25idx) / (p75idx + p25idx);
  double coveredImageRange = (maximum - minimum)/ (absoluteMaximum - absoluteMinimum) ;

  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Mean"),mean));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Variance"),variance));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Skewness"),skewness));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Excess kurtosis"),kurtosis-3));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Median"),median));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Minimum"),minimum));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "05th Percentile"),percentiles[0]));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "10th Percentile"),percentiles[1]));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "15th Percentile"),percentiles[2]));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "20th Percentile"),percentiles[3]));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "25th Percentile"),percentiles[4]));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "30th Percentile"),percentiles[5]));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "35th Percentile"),percentiles[6]));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "40th Percentile"),percentiles[7]));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "45th Percentile"),percentiles[8]));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "50th Percentile"),percentiles[9]));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "55th Percentile"),percentiles[10]));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "60th Percentile"),percentiles[11]));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "65th Percentile"),percentiles[12]));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "70th Percentile"), percentiles[13]));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "75th Percentile"), percentiles[14]));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "80th Percentile"), percentiles[15]));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "85th Percentile"), percentiles[16]));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "90th Percentile"), percentiles[17]));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "95th Percentile"), percentiles[18]));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Maximum"), maximum));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Interquantile range"), interquantileRange));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Range"), maximum-minimum));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Mean absolute deviation"), meanAbsoluteDeviation));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Robust mean absolute deviation"), robustMeanAbsoluteDeviation));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Median absolute deviation"), medianAbsoluteDeviation));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Coefficient of variation"), coefficientOfVariation));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Quantile coefficient of dispersion"), quantileCoefficientOfDispersion));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Energy"), energy));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Root mean square"), rootMeanSquare));

  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Standard Deviation"), std::sqrt(variance)));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Kurtosis"), kurtosis));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Robust mean"), robustMean));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Robust variance"), robustVariance));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Covered image intensity range"), coveredImageRange));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Mode index"), modeIdx));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Mode value"), params.quantifier->IndexToMeanIntensity(modeIdx)));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Mode probability"), histogram[modeIdx] / (1.0*numberOfVoxels)));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Entropy"), entropy));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Uniformtiy"), uniformity));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Number of voxels"), numberOfVoxels));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Sum of voxels"), sum));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Voxel space"), voxelSpace));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Voxel volume"), voxelVolume));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Image Dimension"), VImageDimension));

  return;
}

mitk::GIFFirstOrderNumericStatistics::GIFFirstOrderNumericStatistics()
{
  SetShortName("fon");
  SetLongName("first-order-numeric");
  SetFeatureClassName("First Order Numeric");
}

void mitk::GIFFirstOrderNumericStatistics::AddArguments(mitkCommandLineParser& parser) const
{
  this->AddQuantifierArguments(parser);

  std::string name = this->GetOptionPrefix();
  parser.addArgument(this->GetLongName(), name, mitkCommandLineParser::Bool, "Use first order statistics (numericaly) as feature", "calculates first order statistics based features", us::Any());
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::GIFFirstOrderNumericStatistics::DoCalculateFeatures(const Image* image, const Image* mask)
{
  FeatureListType featureList;

  this->InitializeQuantifier(image, mask);

  MITK_INFO << "Start calculating first order features ....";

  FirstOrderNumericParameterStruct params;
  params.quantifier = GetQuantifier();
  params.MinimumIntensity = GetQuantifier()->GetMinimum();
  params.MaximumIntensity = GetQuantifier()->GetMaximum();
  params.Bins = GetQuantifier()->GetBins();
  params.id = this->CreateTemplateFeatureID();
  AccessByItk_3(image, CalculateFirstOrderStatistics, mask, featureList, params);

  MITK_INFO << "Finished calculating first order features....";

  return featureList;
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::GIFFirstOrderNumericStatistics::CalculateFeatures(const Image* image, const Image*, const Image* maskNoNAN)
{
  return Superclass::CalculateFeatures(image, maskNoNAN);
}

