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
    labelStatisticsImageFilter->SetHistogramParameters(params.m_HistogramSize, minMaxComputer->GetMinimum(),minMaxComputer->GetMaximum());
  }
  labelStatisticsImageFilter->Update();

  // --------------- Range --------------------
  double range = labelStatisticsImageFilter->GetMaximum(1) - labelStatisticsImageFilter->GetMinimum(1);
  // --------------- Uniformity, Entropy --------------------
  double count = labelStatisticsImageFilter->GetCount(1);
  //double std_dev = labelStatisticsImageFilter->GetSigma(1);
  double uncorrected_std_dev = std::sqrt((count - 1) / count * labelStatisticsImageFilter->GetVariance(1));
  double mean = labelStatisticsImageFilter->GetMean(1);
  auto histogram = labelStatisticsImageFilter->GetHistogram(1);
  HIndexType index;
  index.SetSize(1);
  double binWidth = histogram->GetBinMax(0, 0) - histogram->GetBinMin(0, 0);

  double uniformity = 0;
  double entropy = 0;
  double squared_sum = 0;
  double kurtosis = 0;
  double mean_absolut_deviation = 0;
  double skewness = 0;
  double sum_prob = 0;

  double Log2=log(2);
  for (int i = 0; i < (int)(histogram->GetSize(0)); ++i)
  {
    index[0] = i;
    double prob = histogram->GetFrequency(index);

    if (prob < 0.1)
      continue;

    double voxelValue = histogram->GetBinMin(0, i) +binWidth * 0.5;

    sum_prob += prob;
    squared_sum += prob * voxelValue*voxelValue;

    prob /= count;
    mean_absolut_deviation += prob* std::abs(voxelValue - mean);

    kurtosis +=prob* (voxelValue - mean) * (voxelValue - mean) * (voxelValue - mean) * (voxelValue - mean);
    skewness += prob* (voxelValue - mean) * (voxelValue - mean) * (voxelValue - mean);

    uniformity += prob*prob;
    if (prob > 0)
    {
      entropy += prob * std::log(prob) / Log2;
    }
  }

  double rms = std::sqrt(squared_sum / count);
  kurtosis = kurtosis / (uncorrected_std_dev*uncorrected_std_dev * uncorrected_std_dev*uncorrected_std_dev);
  skewness = skewness / (uncorrected_std_dev*uncorrected_std_dev * uncorrected_std_dev);
  //mean_absolut_deviation = mean_absolut_deviation;
  double coveredGrayValueRange = range / imageRange;

  featureList.push_back(std::make_pair("FirstOrder Range",range));
  featureList.push_back(std::make_pair("FirstOrder Uniformity",uniformity));
  featureList.push_back(std::make_pair("FirstOrder Entropy",entropy));
  featureList.push_back(std::make_pair("FirstOrder Energy",squared_sum));
  featureList.push_back(std::make_pair("FirstOrder RMS",rms));
  featureList.push_back(std::make_pair("FirstOrder Kurtosis",kurtosis));
  featureList.push_back(std::make_pair("FirstOrder Skewness",skewness));
  featureList.push_back(std::make_pair("FirstOrder Mean absolute deviation",mean_absolut_deviation));
  featureList.push_back(std::make_pair("FirstOrder Covered Image Intensity Range",coveredGrayValueRange));

  featureList.push_back(std::make_pair("FirstOrder Minimum",labelStatisticsImageFilter->GetMinimum(1)));
  featureList.push_back(std::make_pair("FirstOrder Maximum",labelStatisticsImageFilter->GetMaximum(1)));
  featureList.push_back(std::make_pair("FirstOrder Mean",labelStatisticsImageFilter->GetMean(1)));
  featureList.push_back(std::make_pair("FirstOrder Variance",labelStatisticsImageFilter->GetVariance(1)));
  featureList.push_back(std::make_pair("FirstOrder Sum",labelStatisticsImageFilter->GetSum(1)));
  featureList.push_back(std::make_pair("FirstOrder Median",labelStatisticsImageFilter->GetMedian(1)));
  featureList.push_back(std::make_pair("FirstOrder Standard deviation",labelStatisticsImageFilter->GetSigma(1)));
  featureList.push_back(std::make_pair("FirstOrder No. of Voxel",labelStatisticsImageFilter->GetCount(1)));
}

mitk::GIFFirstOrderStatistics::GIFFirstOrderStatistics() :
  m_HistogramSize(256), m_UseCtRange(false)
{
}

mitk::GIFFirstOrderStatistics::FeatureListType mitk::GIFFirstOrderStatistics::CalculateFeatures(const Image::Pointer & image, const Image::Pointer &mask)
{
  FeatureListType featureList;

  ParameterStruct params;
  params.m_HistogramSize = this->m_HistogramSize;
  params.m_UseCtRange = this->m_UseCtRange;

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