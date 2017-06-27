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

template<typename TPixel, unsigned int VImageDimension>
void
CalculateFirstOrderStatistics(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, mitk::GIFFirstOrderStatistics::FeatureListType & featureList, mitk::GIFFirstOrderStatistics::ParameterStruct params)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<int, VImageDimension> MaskType;
  typedef itk::LabelStatisticsImageFilter<ImageType, MaskType> FilterType;
  typedef typename FilterType::HistogramType HistogramType;
  typedef typename HistogramType::IndexType HIndexType;
  typedef itk::MinimumMaximumImageCalculator<ImageType> MinMaxComputerType;

  typename MaskType::Pointer maskImage = MaskType::New();
  mitk::CastToItkImage(mask, maskImage);

  typename MinMaxComputerType::Pointer minMaxComputer = MinMaxComputerType::New();
  minMaxComputer->SetImage(itkImage);
  minMaxComputer->Compute();
  double imageRange = minMaxComputer->GetMaximum() -  minMaxComputer->GetMinimum();

  typename FilterType::Pointer labelStatisticsImageFilter = FilterType::New();
  labelStatisticsImageFilter->SetInput( itkImage );
  labelStatisticsImageFilter->SetLabelInput(maskImage);
  labelStatisticsImageFilter->SetUseHistograms(true);
  if (params.m_UseCtRange)
  {
    labelStatisticsImageFilter->SetHistogramParameters(1024.5+3096.5, -1024.5,3096.5);
  } else {
    double min = minMaxComputer->GetMinimum() -0.5;
    double max = minMaxComputer->GetMaximum() +0.5;

    if (params.UseMinimumIntensity)
      min = params.MinimumIntensity;
    if (params.UseMaximumIntensity)
      max = params.MaximumIntensity;

    labelStatisticsImageFilter->SetHistogramParameters(params.Bins, min,max);
  }
  labelStatisticsImageFilter->Update();

  // --------------- Range --------------------
  double range = labelStatisticsImageFilter->GetMaximum(1) - labelStatisticsImageFilter->GetMinimum(1);
  // --------------- Uniformity, Entropy --------------------
  double count = labelStatisticsImageFilter->GetCount(1);
  //double std_dev = labelStatisticsImageFilter->GetSigma(1);
  double mean = labelStatisticsImageFilter->GetMean(1);
  auto histogram = labelStatisticsImageFilter->GetHistogram(1);
  bool histogramIsCalculated = histogram;

  HIndexType index;
  index.SetSize(1);


  double uniformity = 0;
  double entropy = 0;
  double squared_sum = 0;
  double kurtosis = 0;
  double mean_absolut_deviation = 0;
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
  //mean_absolut_deviation = mean_absolut_deviation;
  double coveredGrayValueRange = range / imageRange;

  featureList.push_back(std::make_pair("FirstOrder Range",range));
  featureList.push_back(std::make_pair("FirstOrder Uniformity",uniformity));
  featureList.push_back(std::make_pair("FirstOrder Entropy",entropy));
  featureList.push_back(std::make_pair("FirstOrder Energy",squared_sum));
  featureList.push_back(std::make_pair("FirstOrder RMS",rms));
  featureList.push_back(std::make_pair("FirstOrder Kurtosis", kurtosis));
  featureList.push_back(std::make_pair("FirstOrder Excess Kurtosis", kurtosis-3));
  featureList.push_back(std::make_pair("FirstOrder Skewness",skewness));
  featureList.push_back(std::make_pair("FirstOrder Mean absolute deviation",mean_absolut_deviation));
  featureList.push_back(std::make_pair("FirstOrder Covered Image Intensity Range",coveredGrayValueRange));

  featureList.push_back(std::make_pair("FirstOrder Minimum",labelStatisticsImageFilter->GetMinimum(1)));
  featureList.push_back(std::make_pair("FirstOrder Maximum",labelStatisticsImageFilter->GetMaximum(1)));
  featureList.push_back(std::make_pair("FirstOrder Mean",labelStatisticsImageFilter->GetMean(1)));
  featureList.push_back(std::make_pair("FirstOrder Unbiased Variance", labelStatisticsImageFilter->GetVariance(1))); //Siehe Definition von Unbiased Variance estimation. (Wird nicht durch n sondern durch n-1 normalisiert)
  featureList.push_back(std::make_pair("FirstOrder Biased Variance", variance));
  featureList.push_back(std::make_pair("FirstOrder Sum",labelStatisticsImageFilter->GetSum(1)));
  featureList.push_back(std::make_pair("FirstOrder Median", labelStatisticsImageFilter->GetMedian(1)));
  featureList.push_back(std::make_pair("FirstOrder Mode", mode_bin));
  featureList.push_back(std::make_pair("FirstOrder Mode Probability", mode_value));
  featureList.push_back(std::make_pair("FirstOrder Standard deviation",labelStatisticsImageFilter->GetSigma(1)));
  featureList.push_back(std::make_pair("FirstOrder No. of Voxel",labelStatisticsImageFilter->GetCount(1)));

  featureList.push_back(std::make_pair("FirstOrder 05th Percentile", p05th));
  featureList.push_back(std::make_pair("FirstOrder 10th Percentile", p10th));
  featureList.push_back(std::make_pair("FirstOrder 15th Percentile", p15th));
  featureList.push_back(std::make_pair("FirstOrder 20th Percentile", p20th));
  featureList.push_back(std::make_pair("FirstOrder 25th Percentile", p25th));
  featureList.push_back(std::make_pair("FirstOrder 30th Percentile", p30th));
  featureList.push_back(std::make_pair("FirstOrder 35th Percentile", p35th));
  featureList.push_back(std::make_pair("FirstOrder 40th Percentile", p40th));
  featureList.push_back(std::make_pair("FirstOrder 45th Percentile", p45th));
  featureList.push_back(std::make_pair("FirstOrder 50th Percentile", p50th));
  featureList.push_back(std::make_pair("FirstOrder 55th Percentile", p55th));
  featureList.push_back(std::make_pair("FirstOrder 60th Percentile", p60th));
  featureList.push_back(std::make_pair("FirstOrder 65th Percentile", p65th));
  featureList.push_back(std::make_pair("FirstOrder 70th Percentile", p70th));
  featureList.push_back(std::make_pair("FirstOrder 75th Percentile", p75th));
  featureList.push_back(std::make_pair("FirstOrder 80th Percentile", p80th));
  featureList.push_back(std::make_pair("FirstOrder 85th Percentile", p85th));
  featureList.push_back(std::make_pair("FirstOrder 90th Percentile", p90th));
  featureList.push_back(std::make_pair("FirstOrder 95th Percentile", p95th));
  featureList.push_back(std::make_pair("FirstOrder Interquartile Range",(p75th - p25th)));

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
  featureList.push_back(std::make_pair("FirstOrder Robust Mean", meanRobust));
  featureList.push_back(std::make_pair("FirstOrder Robust Mean Absolute Deviation", robustMeanAbsoluteDeviation));


}

mitk::GIFFirstOrderStatistics::GIFFirstOrderStatistics() :
m_HistogramSize(256), m_UseCtRange(false)
{
  SetShortName("fo");
  SetLongName("first-order");
}

mitk::GIFFirstOrderStatistics::FeatureListType mitk::GIFFirstOrderStatistics::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;

  ParameterStruct params;
  params.m_HistogramSize = this->m_HistogramSize;
  params.m_UseCtRange = this->m_UseCtRange;

  params.MinimumIntensity = GetMinimumIntensity();
  params.MaximumIntensity = GetMaximumIntensity();
  params.UseMinimumIntensity = GetUseMinimumIntensity();
  params.UseMaximumIntensity = GetUseMaximumIntensity();
  params.Bins = GetBins();

  AccessByItk_3(image, CalculateFirstOrderStatistics, mask, featureList, params);

  return featureList;
}

mitk::GIFFirstOrderStatistics::FeatureNameListType mitk::GIFFirstOrderStatistics::GetFeatureNames()
{
  FeatureNameListType featureList;
  featureList.push_back("FirstOrder Minimum");
  featureList.push_back("FirstOrder Maximum");
  featureList.push_back("FirstOrder Mean");
  featureList.push_back("FirstOrder Variance");
  featureList.push_back("FirstOrder Sum");
  featureList.push_back("FirstOrder Median");
  featureList.push_back("FirstOrder Standard deviation");
  featureList.push_back("FirstOrder No. of Voxel");
  return featureList;
}


void mitk::GIFFirstOrderStatistics::AddArguments(mitkCommandLineParser &parser)
{
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::String, "Use Volume-Statistic", "calculates volume based features", us::Any());
}

void
mitk::GIFFirstOrderStatistics::CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &, const Image::Pointer &maskNoNAN, FeatureListType &featureList)
{
  auto parsedArgs = GetParameter();
  if (parsedArgs.count(GetLongName()))
  {
    MITK_INFO << "Start calculating first order features ....";
    auto localResults = this->CalculateFeatures(feature, maskNoNAN);
    featureList.insert(featureList.end(), localResults.begin(), localResults.end());
    MITK_INFO << "Finished calculating first order features....";
  }
}

