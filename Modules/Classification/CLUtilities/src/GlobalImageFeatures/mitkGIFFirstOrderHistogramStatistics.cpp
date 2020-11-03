/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkGIFFirstOrderHistogramStatistics.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>

// ITK
#include <itkLabelStatisticsImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkImageRegionConstIteratorWithIndex.h>

// STL
#include <sstream>
#include <limits>
#include <math.h>

namespace mitk
{
  struct GIFFirstOrderHistogramStatisticsConfiguration
  {
    double MinimumIntensity;
    double MaximumIntensity;
    int Bins;
    FeatureID id;
  };
}

#define GET_VARIABLE_INDEX(value) \
   mv[0] = value; \
   histogram->GetIndex(mv, resultingIndex);


template<typename TPixel, unsigned int VImageDimension>
void
CalculateFirstOrderHistogramStatistics(const itk::Image<TPixel, VImageDimension>* itkImage, const mitk::Image* mask, mitk::GIFFirstOrderHistogramStatistics::FeatureListType & featureList, mitk::GIFFirstOrderHistogramStatisticsConfiguration params)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned short, VImageDimension> MaskType;
  typedef itk::LabelStatisticsImageFilter<ImageType, MaskType> FilterType;
  typedef typename FilterType::HistogramType HistogramType;
  typedef typename HistogramType::IndexType HIndexType;

  typename MaskType::Pointer maskImage = MaskType::New();
  mitk::CastToItkImage(mask, maskImage);

  typename FilterType::Pointer labelStatisticsImageFilter = FilterType::New();
  labelStatisticsImageFilter->SetInput(itkImage);
  labelStatisticsImageFilter->SetLabelInput(maskImage);
  labelStatisticsImageFilter->SetUseHistograms(true);
  labelStatisticsImageFilter->SetHistogramParameters(params.Bins, params.MinimumIntensity, params.MaximumIntensity);

  labelStatisticsImageFilter->Update();

  typename HistogramType::MeasurementVectorType mv(1);
  mv[0] = 4.1;
  typename HistogramType::IndexType resultingIndex;

  auto histogram = labelStatisticsImageFilter->GetHistogram(1);
  double meanValue = 0; // labelStatisticsImageFilter->GetMean(1);
  GET_VARIABLE_INDEX(meanValue);
  double meanIndex = 0; // resultingIndex[0];
  double medianValue = labelStatisticsImageFilter->GetMedian(1);
  GET_VARIABLE_INDEX(medianValue);
  double medianIndex = resultingIndex[0];
  double minimumValue = labelStatisticsImageFilter->GetMinimum(1);
  GET_VARIABLE_INDEX(minimumValue);
  double minimumIndex = resultingIndex[0];
  double p10Value = histogram->Quantile(0, 0.10);
  GET_VARIABLE_INDEX(p10Value);
  double p10Index = resultingIndex[0];
  double p25Value = histogram->Quantile(0, 0.25);
  GET_VARIABLE_INDEX(p25Value);
  double p25Index = resultingIndex[0];
  double p75Value = histogram->Quantile(0, 0.75);
  GET_VARIABLE_INDEX(p75Value);
  double p75Index = resultingIndex[0];
  double p90Value = histogram->Quantile(0, 0.90);
  GET_VARIABLE_INDEX(p90Value);
  double p90Index = resultingIndex[0];
  double maximumValue = labelStatisticsImageFilter->GetMaximum(1);
  GET_VARIABLE_INDEX(maximumValue);
  double maximumIndex = resultingIndex[0];

  double Log2 = log(2);
  HIndexType index;
  HIndexType index2;
  index.SetSize(1);
  index2.SetSize(1);
  double binWidth = histogram->GetBinMax(0, 0) - histogram->GetBinMin(0, 0);
  double count = labelStatisticsImageFilter->GetCount(1);

  double robustMeanValue = 0;
  double robustMeanIndex = 0;
  double robustCount = 0;

  for (int i = 0; i < (int)(histogram->GetSize(0)); ++i)
  {
    index[0] = i;
    double frequence = histogram->GetFrequency(index);
    double probability = frequence / count;
    double voxelValue = histogram->GetBinMin(0, i) + binWidth * 0.5;

    meanValue += probability * voxelValue;
    meanIndex += probability * (i);

    if ((i >= p10Index) && (i <= p90Index))
    {
      robustMeanValue += frequence * voxelValue;
      robustMeanIndex += frequence * i;
      robustCount += frequence;
    }
  }
  robustMeanValue /= robustCount;
  robustMeanIndex /= robustCount;

  double varianceValue = 0;
  double varianceIndex = 0;
  double skewnessValue = 0;
  double skewnessIndex = 0;
  double kurtosisValue = 0;
  double kurtosisIndex = 0;
  double modeValue = 0;
  double modeIndex = 0;
  double modeFrequence = 0;
  double meanAbsoluteDeviationValue = 0;
  double meanAbsoluteDeviationIndex = 0;
  double robustMeanAbsoluteDeviationValue = 0;
  double robustMeanAbsoluteDeivationIndex = 0;
  double medianAbsoluteDeviationValue = 0;
  double medianAbsoluteDeviationIndex = 0;
  double coefficientOfVariationValue = 0;
  double coefficientOfVariationIndex = 0;
  double quantileCoefficientOfDispersionValue = 0;
  double quantileCoefficientOfDispersionIndex = 0;
  double entropyValue = 0;
  double entropyIndex = 0;
  double uniformityValue = 0;
  double uniformityIndex = 0;

  double maximumGradientValue = std::numeric_limits<double>::min();
  double maximumGradientIndex = 0;
  double minimumGradientValue = std::numeric_limits<double>::max();
  double minimumGradientIndex = 0;

  double gradient = 0;

  for (int i = 0; i < (int)(histogram->GetSize(0)); ++i)
  {
    index[0] = i;
    double frequence = histogram->GetFrequency(index);
    double probability = frequence / count;
    double voxelValue = histogram->GetBinMin(0, i) + binWidth * 0.5;

    double deltaValue = (voxelValue - meanValue);
    double deltaIndex = (i - meanIndex);

    varianceValue += probability * deltaValue * deltaValue;
    varianceIndex += probability * deltaIndex * deltaIndex;
    skewnessValue += probability * deltaValue * deltaValue * deltaValue;
    skewnessIndex += probability * deltaIndex * deltaIndex * deltaIndex;
    kurtosisValue += probability * deltaValue * deltaValue * deltaValue * deltaValue;
    kurtosisIndex += probability * deltaIndex * deltaIndex * deltaIndex * deltaIndex;

    if (modeFrequence < frequence)
    {
      modeFrequence = frequence;
      modeValue = voxelValue;
      modeIndex = i;
    }
    meanAbsoluteDeviationValue += probability * std::abs(deltaValue);
    meanAbsoluteDeviationIndex += probability * std::abs(deltaIndex);
    if ((i >= p10Index) && (i <= p90Index))
    {
      robustMeanAbsoluteDeviationValue += frequence * std::abs<double>(voxelValue - robustMeanValue);
      robustMeanAbsoluteDeivationIndex += frequence * std::abs<double>(i*1.0 - robustMeanIndex*1.0);
    }
    medianAbsoluteDeviationValue += probability * std::abs(voxelValue - medianValue);
    medianAbsoluteDeviationIndex += probability * std::abs(i*1.0 - medianIndex);
    if (probability > 0.0000001)
    {
      entropyValue -= probability * std::log(probability) / Log2;
      entropyIndex = entropyValue;
    }
    uniformityValue += probability*probability;
    uniformityIndex = uniformityValue;
    if (i == 0)
    {
      index[0] = 1; index2[0] = 0;
      gradient = histogram->GetFrequency(index)*1.0 - histogram->GetFrequency(index2)*1.0;
    }
    else if (i == (int)(histogram->GetSize(0)) - 1)
    {
      index[0] = i; index2[0] = i - 1;
      gradient = histogram->GetFrequency(index)*1.0 - histogram->GetFrequency(index2)*1.0;
    }
    else
    {
      index[0] = i+1; index2[0] = i - 1;
      gradient = (histogram->GetFrequency(index)*1.0 - histogram->GetFrequency(index2)*1.0) / 2.0;
    }
    if (gradient > maximumGradientValue)
    {
      maximumGradientValue = gradient;
      maximumGradientIndex = i + 1;
    }
    if (gradient < minimumGradientValue)
    {
      minimumGradientValue = gradient;
      minimumGradientIndex = i + 1;
    }
  }
  skewnessValue = skewnessValue / (varianceValue * std::sqrt(varianceValue));
  skewnessIndex = skewnessIndex / (varianceIndex * std::sqrt(varianceIndex));
  kurtosisValue = kurtosisValue / (varianceValue * varianceValue) - 3; // Excess Kurtosis
  kurtosisIndex = kurtosisIndex / (varianceIndex * varianceIndex) - 3; // Excess Kurtosis
  coefficientOfVariationValue = std::sqrt(varianceValue) / meanValue;
  coefficientOfVariationIndex = std::sqrt(varianceIndex) / (meanIndex+1);
  quantileCoefficientOfDispersionValue = (p75Value - p25Value) / (p75Value + p25Value);
  quantileCoefficientOfDispersionIndex = (p75Index - p25Index) / (p75Index + 1.0 + p25Index + 1.0);
  robustMeanAbsoluteDeviationValue /= robustCount;
  robustMeanAbsoluteDeivationIndex /= robustCount;

  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Mean Value"), meanValue));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Variance Value"), varianceValue));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Skewness Value"), skewnessValue));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Excess Kurtosis Value"), kurtosisValue));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Median Value"), medianValue));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Minimum Value"), minimumValue));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Percentile 10 Value"), p10Value));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Percentile 90 Value"), p90Value));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Maximum Value"), maximumValue));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Mode Value"), modeValue));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Interquantile Range Value"), p75Value - p25Value));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Range Value"), maximumValue - minimumValue));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Mean Absolute Deviation Value"), meanAbsoluteDeviationValue));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Robust Mean Absolute Deviation Value"), robustMeanAbsoluteDeviationValue));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Median Absolute Deviation Value"), medianAbsoluteDeviationValue));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Coefficient of Variation Value"), coefficientOfVariationValue));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Quantile coefficient of Dispersion Value"), quantileCoefficientOfDispersionValue));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Entropy Value"), entropyValue));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Uniformity Value"), uniformityValue));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Robust Mean Value"), robustMeanValue));

  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Mean Index"), meanIndex + 1 ));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Variance Index"), varianceIndex));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Skewness Index"), skewnessIndex));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Excess Kurtosis Index"), kurtosisIndex));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Median Index"), medianIndex + 1));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Minimum Index"), minimumIndex + 1));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Percentile 10 Index"), p10Index + 1));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Percentile 90 Index"), p90Index + 1));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Maximum Index"), maximumIndex + 1));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Mode Index"), modeIndex + 1));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Interquantile Range Index"), p75Index - p25Index));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Range Index"), maximumIndex - minimumIndex));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Mean Absolute Deviation Index"), meanAbsoluteDeviationIndex));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Robust Mean Absolute Deviation Index"), robustMeanAbsoluteDeivationIndex));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Median Absolute Deviation Index"), medianAbsoluteDeviationIndex));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Coefficient of Variation Index"), coefficientOfVariationIndex));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Quantile coefficient of Dispersion Index"), quantileCoefficientOfDispersionIndex));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Entropy Index"), entropyIndex));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Uniformity Index"), uniformityIndex));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Maximum Gradient"), maximumGradientValue));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Maximum Gradient Index"), maximumGradientIndex));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Minimum Gradient"), minimumGradientValue));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Minimum Gradient Index"), minimumGradientIndex));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Robust Mean Index"), robustMeanIndex));

  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Number of Bins"), histogram->GetSize(0)));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id, "Bin Size"), binWidth));
}

mitk::GIFFirstOrderHistogramStatistics::GIFFirstOrderHistogramStatistics()
{
  SetShortName("foh");
  SetLongName("first-order-histogram");
  SetFeatureClassName("First Order Histogram");
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::GIFFirstOrderHistogramStatistics::DoCalculateFeatures(const Image* image, const Image* mask)
{
  FeatureListType featureList;

  this->InitializeQuantifier(image, mask);

  MITK_INFO << "Start calculating first order histogram features ....";

  GIFFirstOrderHistogramStatisticsConfiguration config;
  config.MinimumIntensity = GetQuantifier()->GetMinimum();
  config.MaximumIntensity = GetQuantifier()->GetMaximum();
  config.Bins = GetQuantifier()->GetBins();
  config.id = this->CreateTemplateFeatureID();

  AccessByItk_3(image, CalculateFirstOrderHistogramStatistics, mask, featureList, config);

  MITK_INFO << "Finished calculating first order histogram features....";

  return featureList;
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::GIFFirstOrderHistogramStatistics::CalculateFeatures(const Image* image, const Image*, const Image* maskNoNAN)
{
  return Superclass::CalculateFeatures(image, maskNoNAN);
}

void mitk::GIFFirstOrderHistogramStatistics::AddArguments(mitkCommandLineParser& parser) const
{
  this->AddQuantifierArguments(parser);

  std::string name = this->GetOptionPrefix();

  parser.addArgument(this->GetLongName(), name, mitkCommandLineParser::Bool, "Use Co-occurence matrix", "calculates Co-occurence based features (new implementation)", us::Any());
}

