/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkGIFFirstOrderStatistics.h>

// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>

// ITK
#include <itkLabelStatisticsImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>

// STL
#include <sstream>

namespace mitk
{
  struct GIFFirstOrderStatisticsParameterStruct
  {
    double MinimumIntensity;
    double MaximumIntensity;
    int Bins;
    FeatureID id;
  };
}

template<typename TPixel, unsigned int VImageDimension>
void
CalculateFirstOrderStatistics(const itk::Image<TPixel, VImageDimension>* itkImage, const mitk::Image* mask, mitk::GIFFirstOrderStatistics::FeatureListType & featureList, mitk::GIFFirstOrderStatisticsParameterStruct params)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned short, VImageDimension> MaskType;
  typedef itk::LabelStatisticsImageFilter<ImageType, MaskType> FilterType;
  typedef typename FilterType::HistogramType HistogramType;
  typedef typename HistogramType::IndexType HIndexType;
  typedef itk::MinimumMaximumImageCalculator<ImageType> MinMaxComputerType;

  typename MaskType::Pointer maskImage = MaskType::New();
  mitk::CastToItkImage(mask, maskImage);

  double voxelVolume = 1;
  for (unsigned int i = 0; i < std::min<unsigned int>(3, VImageDimension); ++i)
    voxelVolume *= itkImage->GetSpacing()[i];
  double voxelSpace = 1;
  for (unsigned int i = 0; i < VImageDimension; ++i)
    voxelSpace *= itkImage->GetSpacing()[i];

  typename MinMaxComputerType::Pointer minMaxComputer = MinMaxComputerType::New();
  minMaxComputer->SetImage(itkImage);
  minMaxComputer->Compute();
  double imageRange = minMaxComputer->GetMaximum() -  minMaxComputer->GetMinimum();

  typename FilterType::Pointer labelStatisticsImageFilter = FilterType::New();
  labelStatisticsImageFilter->SetInput( itkImage );
  labelStatisticsImageFilter->SetLabelInput(maskImage);
  labelStatisticsImageFilter->SetUseHistograms(true);

  double min = params.MinimumIntensity;
  double max = params.MaximumIntensity;

  labelStatisticsImageFilter->SetHistogramParameters(params.Bins, min,max);
  labelStatisticsImageFilter->Update();

  // --------------- Range --------------------
  double range = labelStatisticsImageFilter->GetMaximum(1) - labelStatisticsImageFilter->GetMinimum(1);
  // --------------- Uniformity, Entropy --------------------
  double count = labelStatisticsImageFilter->GetCount(1);
  //double std_dev = labelStatisticsImageFilter->GetSigma(1);
  double mean = labelStatisticsImageFilter->GetMean(1);
  double median = labelStatisticsImageFilter->GetMedian(1);
  auto histogram = labelStatisticsImageFilter->GetHistogram(1);
  bool histogramIsCalculated = histogram;

  HIndexType index;
  index.SetSize(1);

  double uniformity = 0;
  double entropy = 0;
  double squared_sum = 0;
  double kurtosis = 0;
  double mean_absolut_deviation = 0;
  double median_absolut_deviation = 0;
  double skewness = 0;
  double sum_prob = 0;
  double binWidth = 0;
  double p05th = 0, p10th = 0, p15th = 0, p20th = 0, p25th = 0, p30th = 0, p35th = 0, p40th = 0, p45th = 0, p50th = 0;
  double p55th = 0, p60th = 0, p65th = 0, p70th = 0, p75th = 0, p80th = 0, p85th = 0, p90th = 0, p95th = 0;

  double voxelValue = 0;

  if (histogramIsCalculated)
  {
    binWidth = histogram->GetBinMax(0, 0) - histogram->GetBinMin(0, 0);
    p05th = histogram->Quantile(0, 0.05);
    p10th = histogram->Quantile(0, 0.10);
    p15th = histogram->Quantile(0, 0.15);
    p20th = histogram->Quantile(0, 0.20);
    p25th = histogram->Quantile(0, 0.25);
    p30th = histogram->Quantile(0, 0.30);
    p35th = histogram->Quantile(0, 0.35);
    p40th = histogram->Quantile(0, 0.40);
    p45th = histogram->Quantile(0, 0.45);
    p50th = histogram->Quantile(0, 0.50);
    p55th = histogram->Quantile(0, 0.55);
    p60th = histogram->Quantile(0, 0.60);
    p65th = histogram->Quantile(0, 0.65);
    p70th = histogram->Quantile(0, 0.70);
    p75th = histogram->Quantile(0, 0.75);
    p80th = histogram->Quantile(0, 0.80);
    p85th = histogram->Quantile(0, 0.85);
    p90th = histogram->Quantile(0, 0.90);
    p95th = histogram->Quantile(0, 0.95);
  }
  double Log2=log(2);
  double mode_bin;
  double mode_value = 0;
  double variance = 0;
  if (histogramIsCalculated)
  {
    for (int i = 0; i < (int)(histogram->GetSize(0)); ++i)
    {
      index[0] = i;
      double prob = histogram->GetFrequency(index);


      if (prob < 0.00000001)
        continue;

      voxelValue = histogram->GetBinMin(0, i) + binWidth * 0.5;

      if (prob > mode_value)
      {
        mode_value = prob;
        mode_bin = voxelValue;
      }

      sum_prob += prob;
      squared_sum += prob * voxelValue*voxelValue;

      prob /= count;
      mean_absolut_deviation += prob* std::abs(voxelValue - mean);
      median_absolut_deviation += prob* std::abs(voxelValue - median);
      variance += prob * (voxelValue - mean) * (voxelValue - mean);

      kurtosis += prob* (voxelValue - mean) * (voxelValue - mean) * (voxelValue - mean) * (voxelValue - mean);
      skewness += prob* (voxelValue - mean) * (voxelValue - mean) * (voxelValue - mean);

      uniformity += prob*prob;
      if (prob > 0)
      {
        entropy += prob * std::log(prob) / Log2;
      }
    }
  }
  entropy = -entropy;

  double uncorrected_std_dev = std::sqrt(variance);
  double rms = std::sqrt(squared_sum / count);
  kurtosis = kurtosis / (variance * variance);
  skewness = skewness / (variance * uncorrected_std_dev);
  double coveredGrayValueRange = range / imageRange;
  double coefficient_of_variation = (mean == 0) ? 0 : std::sqrt(variance) / mean;
  double quantile_coefficient_of_dispersion = (p75th - p25th) / (p75th + p25th);

  //Calculate the robust mean absolute deviation
  //First, set all frequencies to 0 that are <10th or >90th percentile
  double meanRobust = 0.0;
  double robustMeanAbsoluteDeviation = 0.0;
  if (histogramIsCalculated)
  {
    for (int i = 0; i < (int)(histogram->GetSize(0)); ++i)
    {
      index[0] = i;
      if (histogram->GetBinMax(0, i) < p10th)
      {
        histogram->SetFrequencyOfIndex(index, 0);
      }
      else if (histogram->GetBinMin(0, i) > p90th)
      {
        histogram->SetFrequencyOfIndex(index, 0);
      }
    }

    //Calculate the mean
    for (int i = 0; i < (int)(histogram->GetSize(0)); ++i)
    {
      index[0] = i;
      meanRobust += histogram->GetFrequency(index) * 0.5 * (histogram->GetBinMin(0, i) + histogram->GetBinMax(0, i));
    }
    meanRobust = meanRobust / histogram->GetTotalFrequency();
    for (int i = 0; i < (int)(histogram->GetSize(0)); ++i)
    {
      index[0] = i;
      robustMeanAbsoluteDeviation += std::abs(histogram->GetFrequency(index) *
        ((0.5 * (histogram->GetBinMin(0, i) + histogram->GetBinMax(0, i)))
        - meanRobust
        ));
    }
    robustMeanAbsoluteDeviation = robustMeanAbsoluteDeviation / histogram->GetTotalFrequency();
  }

  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Mean"), labelStatisticsImageFilter->GetMean(1)));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Unbiased Variance"), labelStatisticsImageFilter->GetVariance(1))); //Siehe Definition von Unbiased Variance estimation. (Wird nicht durch n sondern durch n-1 normalisiert)
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Biased Variance"), variance));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Skewness"), skewness));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Kurtosis"), kurtosis));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Median"), labelStatisticsImageFilter->GetMedian(1)));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Minimum"), labelStatisticsImageFilter->GetMinimum(1)));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Maximum"), labelStatisticsImageFilter->GetMaximum(1)));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Range"), range));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Mean Absolute Deviation"), mean_absolut_deviation));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Robust Mean Absolute Deviation"), robustMeanAbsoluteDeviation));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Median Absolute Deviation"), median_absolut_deviation));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Coefficient Of Variation"), coefficient_of_variation));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Quantile Coefficient Of Dispersion"), quantile_coefficient_of_dispersion));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Energy"), squared_sum));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Root Mean Square"), rms));

  typename HistogramType::MeasurementVectorType mv(1);
  mv[0] = 0;
  typename HistogramType::IndexType resultingIndex;
  histogram->GetIndex(mv, resultingIndex);
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Robust Mean"), meanRobust));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Uniformity"), uniformity));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Entropy"), entropy));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Excess Kurtosis"), kurtosis - 3));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Covered Image Intensity Range"), coveredGrayValueRange));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Sum"), labelStatisticsImageFilter->GetSum(1)));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Mode"), mode_bin));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Mode Probability"), mode_value));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Unbiased Standard deviation"), labelStatisticsImageFilter->GetSigma(1)));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Biased Standard deviation"), sqrt(variance)));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Number Of Voxels"), labelStatisticsImageFilter->GetCount(1)));

  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"05th Percentile"), p05th));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"10th Percentile"), p10th));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"15th Percentile"), p15th));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"20th Percentile"), p20th));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"25th Percentile"), p25th));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"30th Percentile"), p30th));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"35th Percentile"), p35th));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"40th Percentile"), p40th));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"45th Percentile"), p45th));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"50th Percentile"), p50th));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"55th Percentile"), p55th));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"60th Percentile"), p60th));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"65th Percentile"), p65th));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"70th Percentile"), p70th));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"75th Percentile"), p75th));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"80th Percentile"), p80th));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"85th Percentile"), p85th));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"90th Percentile"), p90th));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"95th Percentile"), p95th));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Interquartile Range"), (p75th - p25th)));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Image Dimension"), VImageDimension));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Voxel Space"), voxelSpace));
  featureList.push_back(std::make_pair(mitk::CreateFeatureID(params.id,"Voxel Volume"), voxelVolume));
}

mitk::GIFFirstOrderStatistics::GIFFirstOrderStatistics()
{
  SetShortName("fo");
  SetLongName("first-order");
  SetFeatureClassName("First Order");
}

void mitk::GIFFirstOrderStatistics::AddArguments(mitkCommandLineParser& parser) const
{
  this->AddQuantifierArguments(parser);

  std::string name = this->GetOptionPrefix();
  parser.addArgument(this->GetLongName(), name, mitkCommandLineParser::Bool, "Use first order statistics as feature", "calculates first order statistics features", us::Any());
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::GIFFirstOrderStatistics::DoCalculateFeatures(const Image* image, const Image* mask)
{
  FeatureListType featureList;

  this->InitializeQuantifier(image, mask);

  MITK_INFO << "Start calculating first order features ....";

  GIFFirstOrderStatisticsParameterStruct params;
  params.MinimumIntensity = GetQuantifier()->GetMinimum();
  params.MaximumIntensity = GetQuantifier()->GetMaximum();
  params.Bins = GetQuantifier()->GetBins();
  params.id = this->CreateTemplateFeatureID();
  AccessByItk_3(image, CalculateFirstOrderStatistics, mask, featureList, params);

  MITK_INFO << "Finished calculating first order features....";

  return featureList;
}

mitk::AbstractGlobalImageFeature::FeatureListType mitk::GIFFirstOrderStatistics::CalculateFeatures(const Image* image, const Image*, const Image* maskNoNAN)
{
  return Superclass::CalculateFeatures(image, maskNoNAN);
}
