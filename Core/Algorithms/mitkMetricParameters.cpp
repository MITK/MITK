/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 9502 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkMetricParameters.h"


namespace mitk {

  MetricParameters* MetricParameters::m_Instance = NULL;

  /**
   * Get the instance of this MetricParameters
   */
  MetricParameters* MetricParameters::GetInstance()
  {
    if (m_Instance == NULL)
    {
      m_Instance = new MetricParameters();
    }
    return m_Instance;
  }

  MetricParameters::MetricParameters()
  {
    m_Metric = MEANSQUARESIMAGETOIMAGEMETRIC;
    m_ComputeGradient = true;
    // for itk::KullbackLeiblerCompareHistogramImageToImageMetric
    m_NumberOfHistogramBinsKullbackLeiblerCompareHistogram = 256;
    // for itk::CorrelationCoefficientHistogramImageToImageMetric
    m_NumberOfHistogramBinsCorrelationCoefficientHistogram = 256;
    // for itk::MeanSquaresHistogramImageToImageMetric
    m_NumberOfHistogramBinsMeanSquaresHistogram = 256;
    // for itk::MutualInformationHistogramImageToImageMetric
    m_NumberOfHistogramBinsMutualInformationHistogram = 256;
    // for itk::NormalizedMutualInformationHistogramImageToImageMetric
    m_NumberOfHistogramBinsNormalizedMutualInformationHistogram = 256;
    // for itk::MattesMutualInformationImageToImageMetric
    m_NumberOfHistogramBinsMattesMutualInformation = 256;
    m_UseSamplingMattesMutualInformation = true;
    m_NumberOfSpatialSamplesMattesMutualInformation = 500;
    // for itk::MeanReciprocalSquareDifferenceImageToImageMetric
    m_LambdaMeanReciprocalSquareDifference = 10;
    // for itk::MutualInformationImageToImageMetric
    m_NumberOfSpatialSamplesMutualInformation = 100;
    m_FixedImageStandardDeviationMutualInformation = 0.4;
    m_MovingImageStandardDeviationMutualInformation = 0.4;
    m_UseNormalizer = true;
    m_FixedSmootherVariance = 0.2;
    m_MovingSmootherVariance = 0.2;
  }

  MetricParameters::~MetricParameters()
  {
  }

  void MetricParameters::SetMetric(int metric)
  {
	  m_Metric = metric;
  }

  int MetricParameters::GetMetric()
  {
	  return m_Metric;
  }

  void MetricParameters::SetComputeGradient(bool gradient)
  {
	  m_ComputeGradient = gradient;
  }

  bool MetricParameters::GetComputeGradient()
  {
	  return m_ComputeGradient;
  }

  /********************************** for itk::MeanSquaresImageToImageMetric ****************************************/


  /********************************** for itk::NormalizedCorrelationImageToImageMetric ****************************************/


  /********************************** for itk::GradientDifferenceImageToImageMetric ****************/


  /********************************** for itk::KullbackLeiblerCompareHistogramImageToImageMetric *************************************************/

  void MetricParameters::SetNumberOfHistogramBinsKullbackLeiblerCompareHistogram(unsigned int bins)
  {
	  m_NumberOfHistogramBinsKullbackLeiblerCompareHistogram = bins;
  }

  unsigned int MetricParameters::GetNumberOfHistogramBinsKullbackLeiblerCompareHistogram()
  {
	  return m_NumberOfHistogramBinsKullbackLeiblerCompareHistogram;
  }

  /********************************** for itk::CorrelationCoefficientHistogramImageToImageMetric *********************************/
  
  void MetricParameters::SetNumberOfHistogramBinsCorrelationCoefficientHistogram(unsigned int bins)
  {
	  m_NumberOfHistogramBinsCorrelationCoefficientHistogram = bins;
  }

  unsigned int MetricParameters::GetNumberOfHistogramBinsCorrelationCoefficientHistogram()
  {
	  return m_NumberOfHistogramBinsCorrelationCoefficientHistogram;
  }

  /********************************** for itk::MeanSquaresHistogramImageToImageMetric *********************************/
  
  void MetricParameters::SetNumberOfHistogramBinsMeanSquaresHistogram(unsigned int bins)
  {
	  m_NumberOfHistogramBinsMeanSquaresHistogram = bins;
  }

  unsigned int MetricParameters::GetNumberOfHistogramBinsMeanSquaresHistogram()
  {
	  return m_NumberOfHistogramBinsMeanSquaresHistogram;
  }
  
  /********************************** for itk::MutualInformationHistogramImageToImageMetric *********************************/

  void MetricParameters::SetNumberOfHistogramBinsMutualInformationHistogram(unsigned int bins)
  {
	  m_NumberOfHistogramBinsMutualInformationHistogram = bins;
  }

  unsigned int MetricParameters::GetNumberOfHistogramBinsMutualInformationHistogram()
  {
	  return m_NumberOfHistogramBinsMutualInformationHistogram;
  }
 
  /********************************** for itk::NormalizedMutualInformationHistogramImageToImageMetric *********************************/

  void MetricParameters::SetNumberOfHistogramBinsNormalizedMutualInformationHistogram(unsigned int bins)
  {
	  m_NumberOfHistogramBinsNormalizedMutualInformationHistogram = bins;
  }

  unsigned int MetricParameters::GetNumberOfHistogramBinsNormalizedMutualInformationHistogram()
  {
	  return m_NumberOfHistogramBinsNormalizedMutualInformationHistogram;
  }

  /********************************** for itk::MattesMutualInformationImageToImageMetric *********************************/
  
  void MetricParameters::SetSpatialSamplesMattesMutualInformation(unsigned int spatialSamples)
  {
    m_NumberOfSpatialSamplesMattesMutualInformation = spatialSamples;
  }
  
  unsigned int MetricParameters::GetSpatialSamplesMattesMutualInformation()
  {
    return m_NumberOfSpatialSamplesMattesMutualInformation;
  }
  
  void MetricParameters::SetUseSamplesMattesMutualInformation(bool samples)
  {
    m_UseSamplingMattesMutualInformation = samples;
  }

  bool MetricParameters::GetUseSamplesMattesMutualInformation()
  {
    return m_UseSamplingMattesMutualInformation;
  }

  void MetricParameters::SetNumberOfHistogramBinsMattesMutualInformation(unsigned int bins)
  {
    m_NumberOfHistogramBinsMattesMutualInformation = bins;
  }

  unsigned int MetricParameters::GetNumberOfHistogramBinsMattesMutualInformation()
  {
    return m_NumberOfHistogramBinsMattesMutualInformation;
  }

  /********************************** for itk::MeanReciprocalSquareDifferenceImageToImageMetric ****************************************/
  
  void MetricParameters::SetLambdaMeanReciprocalSquareDifference(unsigned int lambda)
  {
    m_LambdaMeanReciprocalSquareDifference = lambda;
  }

  unsigned int MetricParameters::GetLambdaMeanReciprocalSquareDifference()
  {
    return m_LambdaMeanReciprocalSquareDifference;
  }

  /********************************** for itk::MutualInformationImageToImageMetric ****************************************/

  void MetricParameters::SetSpatialSamplesMutualInformation(unsigned int spatialSamples)
  {
    m_NumberOfSpatialSamplesMutualInformation = spatialSamples;
  }
  
  unsigned int MetricParameters::GetSpatialSamplesMutualInformation()
  {
    return m_NumberOfSpatialSamplesMutualInformation;
  }

  void MetricParameters::SetFixedImageStandardDeviationMutualInformation(float fixedStandardDev)
  {
    m_FixedImageStandardDeviationMutualInformation = fixedStandardDev;
  }
  
  float MetricParameters::GetFixedImageStandardDeviationMutualInformation()
  {
    return m_FixedImageStandardDeviationMutualInformation;
  }

  void MetricParameters::SetMovingImageStandardDeviationMutualInformation(float movingStandardDev)
  {
    m_MovingImageStandardDeviationMutualInformation = movingStandardDev;
  }
  
  float MetricParameters::GetMovingImageStandardDeviationMutualInformation()
  {
    return m_MovingImageStandardDeviationMutualInformation;
  }

  void MetricParameters::SetUseNormalizerAndSmootherMutualInformation(bool useNormalizer)
  {
    m_UseNormalizer = useNormalizer;
  }

  bool MetricParameters::GetUseNormalizerAndSmootherMutualInformation()
  {
    return m_UseNormalizer;
  }

  void MetricParameters::SetFixedSmootherVarianceMutualInformation(float fixedSmootherVariance)
  {
    m_FixedSmootherVariance = fixedSmootherVariance;
  }

  float MetricParameters::GetFixedSmootherVarianceMutualInformation()
  {
    return m_FixedSmootherVariance;
  }

  void MetricParameters::SetMovingSmootherVarianceMutualInformation(float movingSmootherVariance)
  {
    m_MovingSmootherVariance = movingSmootherVariance;
  }

  float MetricParameters::GetMovingSmootherVarianceMutualInformation()
  {
    return m_MovingSmootherVariance;
  }

  /********************************** for itk::MatchCardinalityImageToImageMetric ****************************************************/
  

  /********************************** for itk::KappaStatisticImageToImageMetric ****************************************************/
  

} // namespace mitk
