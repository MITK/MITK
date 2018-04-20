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
  std::string prefix;
};

template<typename TPixel, unsigned int VImageDimension>
void
CalculateFirstOrderStatistics(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask, mitk::GIFFirstOrderNumericStatistics::FeatureListType & featureList, FirstOrderNumericParameterStruct params)
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
  double sum = 0;
  double sumTwo= 0;
  double sumThree = 0;
  unsigned int numberOfVoxels = 0;

  itk::ImageRegionIterator<ImageType> imageIter(itkImage, itkImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<MaskType>  maskIter(maskImage, maskImage->GetLargestPossibleRegion());

  while (!imageIter.IsAtEnd())
  {
    if (maskIter.Get() > 0)
    {
      double value = imageIter.Get();

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
  bool found10th = false;
  bool found90th = false;
  bool foundMedian = false;
  double median = 0;
  for (std::size_t idx = 0; idx < histogram.size(); ++idx)
  {

  }


  double mean = sum / numberOfVoxels;
  double variance = sumTwo / numberOfVoxels - (mean*mean);
  double skewness = (sumThree / numberOfVoxels - 3 * mean * variance - mean * mean * mean) / (std::pow<double>(variance, 3/2 ));
  double energy = sumTwo;
  double rootMeanSquare = std::sqrt(sumTwo / numberOfVoxels);

  double sumAbsoluteDistanceToMean = 0;

  maskIter.GoToBegin();
  imageIter.GoToBegin();
  while (!imageIter.IsAtEnd())
  {
    if (maskIter.Get() > 0)
    {
      double value = imageIter.Get();

      sumAbsoluteDistanceToMean += std::abs<double>(value - mean);

    }
    ++maskIter;
    ++imageIter;
  }
  double meanAbsoluteDeviation = sumAbsoluteDistanceToMean / numberOfVoxels;


  featureList.push_back(std::make_pair(params.prefix + "Mean", mean));
  featureList.push_back(std::make_pair(params.prefix + "Variance", variance));
  featureList.push_back(std::make_pair(params.prefix + "Skewness", skewness));
  featureList.push_back(std::make_pair(params.prefix + "Minimum", minimum));
  featureList.push_back(std::make_pair(params.prefix + "Maximum", maximum));
  featureList.push_back(std::make_pair(params.prefix + "Range", maximum-minimum));
  featureList.push_back(std::make_pair(params.prefix + "Mean absolute deviation", meanAbsoluteDeviation));

  featureList.push_back(std::make_pair(params.prefix + "Energy", energy));
  featureList.push_back(std::make_pair(params.prefix + "Root mean square", rootMeanSquare));

  return;






  /*

  typedef itk::LabelStatisticsImageFilter<ImageType, MaskType> FilterType;
  typedef typename FilterType::HistogramType HistogramType;
  typedef typename HistogramType::IndexType HIndexType;
  typedef itk::MinimumMaximumImageCalculator<ImageType> MinMaxComputerType;




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

  featureList.push_back(std::make_pair(params.prefix + "Mean", labelStatisticsImageFilter->GetMean(1)));
  featureList.push_back(std::make_pair(params.prefix + "Unbiased Variance", labelStatisticsImageFilter->GetVariance(1))); //Siehe Definition von Unbiased Variance estimation. (Wird nicht durch n sondern durch n-1 normalisiert)
  featureList.push_back(std::make_pair(params.prefix + "Biased Variance", variance));
  featureList.push_back(std::make_pair(params.prefix + "Skewness", skewness));
  featureList.push_back(std::make_pair(params.prefix + "Kurtosis", kurtosis));
  featureList.push_back(std::make_pair(params.prefix + "Median", labelStatisticsImageFilter->GetMedian(1)));
  featureList.push_back(std::make_pair(params.prefix + "Minimum", labelStatisticsImageFilter->GetMinimum(1)));
  featureList.push_back(std::make_pair(params.prefix + "Maximum", labelStatisticsImageFilter->GetMaximum(1)));
  featureList.push_back(std::make_pair(params.prefix + "Range", range));
  featureList.push_back(std::make_pair(params.prefix + "Mean Absolute Deviation", mean_absolut_deviation));
  featureList.push_back(std::make_pair(params.prefix + "Robust Mean Absolute Deviation", robustMeanAbsoluteDeviation));
  featureList.push_back(std::make_pair(params.prefix + "Median Absolute Deviation", median_absolut_deviation));
  featureList.push_back(std::make_pair(params.prefix + "Coefficient Of Variation", coefficient_of_variation));
  featureList.push_back(std::make_pair(params.prefix + "Quantile Coefficient Of Dispersion", quantile_coefficient_of_dispersion));
  featureList.push_back(std::make_pair(params.prefix + "Energy", squared_sum));
  featureList.push_back(std::make_pair(params.prefix + "Root Mean Square", rms));

  typename HistogramType::MeasurementVectorType mv(1);
  mv[0] = 0;
  typename HistogramType::IndexType resultingIndex;
  histogram->GetIndex(mv, resultingIndex);
  featureList.push_back(std::make_pair(params.prefix + "Robust Mean", meanRobust));
  featureList.push_back(std::make_pair(params.prefix + "Uniformity", uniformity));
  featureList.push_back(std::make_pair(params.prefix + "Entropy", entropy));
  featureList.push_back(std::make_pair(params.prefix + "Excess Kurtosis", kurtosis - 3));
  featureList.push_back(std::make_pair(params.prefix + "Covered Image Intensity Range", coveredGrayValueRange));
  featureList.push_back(std::make_pair(params.prefix + "Sum", labelStatisticsImageFilter->GetSum(1)));
  featureList.push_back(std::make_pair(params.prefix + "Mode", mode_bin));
  featureList.push_back(std::make_pair(params.prefix + "Mode Probability", mode_value));
  featureList.push_back(std::make_pair(params.prefix + "Unbiased Standard deviation", labelStatisticsImageFilter->GetSigma(1)));
  featureList.push_back(std::make_pair(params.prefix + "Biased Standard deviation", sqrt(variance)));
  featureList.push_back(std::make_pair(params.prefix + "Number Of Voxels", labelStatisticsImageFilter->GetCount(1)));

  featureList.push_back(std::make_pair(params.prefix + "05th Percentile", p05th));
  featureList.push_back(std::make_pair(params.prefix + "10th Percentile", p10th));
  featureList.push_back(std::make_pair(params.prefix + "15th Percentile", p15th));
  featureList.push_back(std::make_pair(params.prefix + "20th Percentile", p20th));
  featureList.push_back(std::make_pair(params.prefix + "25th Percentile", p25th));
  featureList.push_back(std::make_pair(params.prefix + "30th Percentile", p30th));
  featureList.push_back(std::make_pair(params.prefix + "35th Percentile", p35th));
  featureList.push_back(std::make_pair(params.prefix + "40th Percentile", p40th));
  featureList.push_back(std::make_pair(params.prefix + "45th Percentile", p45th));
  featureList.push_back(std::make_pair(params.prefix + "50th Percentile", p50th));
  featureList.push_back(std::make_pair(params.prefix + "55th Percentile", p55th));
  featureList.push_back(std::make_pair(params.prefix + "60th Percentile", p60th));
  featureList.push_back(std::make_pair(params.prefix + "65th Percentile", p65th));
  featureList.push_back(std::make_pair(params.prefix + "70th Percentile", p70th));
  featureList.push_back(std::make_pair(params.prefix + "75th Percentile", p75th));
  featureList.push_back(std::make_pair(params.prefix + "80th Percentile", p80th));
  featureList.push_back(std::make_pair(params.prefix + "85th Percentile", p85th));
  featureList.push_back(std::make_pair(params.prefix + "90th Percentile", p90th));
  featureList.push_back(std::make_pair(params.prefix + "95th Percentile", p95th));
  featureList.push_back(std::make_pair(params.prefix + "Interquartile Range", (p75th - p25th)));
  featureList.push_back(std::make_pair(params.prefix + "Image Dimension", VImageDimension));
  featureList.push_back(std::make_pair(params.prefix + "Voxel Space", voxelSpace));
  featureList.push_back(std::make_pair(params.prefix + "Voxel Volume", voxelVolume)); */
}

mitk::GIFFirstOrderNumericStatistics::GIFFirstOrderNumericStatistics()
{
  SetShortName("fon");
  SetLongName("first-order-numeric");
  SetFeatureClassName("First Order Numeric");
}

mitk::GIFFirstOrderNumericStatistics::FeatureListType mitk::GIFFirstOrderNumericStatistics::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  InitializeQuantifier(image, mask);
  FeatureListType featureList;

  FirstOrderNumericParameterStruct params;

  params.quantifier = GetQuantifier();
  params.MinimumIntensity = GetQuantifier()->GetMinimum();
  params.MaximumIntensity = GetQuantifier()->GetMaximum();
  params.Bins = GetQuantifier()->GetBins();
  params.prefix = FeatureDescriptionPrefix();
  AccessByItk_3(image, CalculateFirstOrderStatistics, mask, featureList, params);

  return featureList;
}

mitk::GIFFirstOrderNumericStatistics::FeatureNameListType mitk::GIFFirstOrderNumericStatistics::GetFeatureNames()
{
  FeatureNameListType featureList;
  return featureList;
}


void mitk::GIFFirstOrderNumericStatistics::AddArguments(mitkCommandLineParser &parser)
{
  std::string name = GetOptionPrefix();

  parser.addArgument(GetLongName(), name, mitkCommandLineParser::Bool, "Use First Order Statistic (Numeric)", "calculates First Order Statistic (Numeric)", us::Any());
  AddQuantifierArguments(parser);
}

void
mitk::GIFFirstOrderNumericStatistics::CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &, const Image::Pointer &maskNoNAN, FeatureListType &featureList)
{
  auto parsedArgs = GetParameter();
  if (parsedArgs.count(GetLongName()))
  {
    InitializeQuantifierFromParameters(feature, maskNoNAN);
    MITK_INFO << "Start calculating first order features ....";
    auto localResults = this->CalculateFeatures(feature, maskNoNAN);
    featureList.insert(featureList.end(), localResults.begin(), localResults.end());
    MITK_INFO << "Finished calculating first order features....";
  }
}

std::string mitk::GIFFirstOrderNumericStatistics::GetCurrentFeatureEncoding()
{
  return QuantifierParameterString();
}