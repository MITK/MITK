/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

  MetricParameters::MetricParameters() : 
    m_Metric(MEANSQUARESIMAGETOIMAGEMETRIC),
    m_ComputeGradient(true),
    // for itk::KullbackLeiblerCompareHistogramImageToImageMetric
    m_NumberOfHistogramBinsKullbackLeiblerCompareHistogram(256),
    // for itk::CorrelationCoefficientHistogramImageToImageMetric
    m_NumberOfHistogramBinsCorrelationCoefficientHistogram(256),
    // for itk::MeanSquaresHistogramImageToImageMetric
    m_NumberOfHistogramBinsMeanSquaresHistogram(256),
    // for itk::MutualInformationHistogramImageToImageMetric
    m_NumberOfHistogramBinsMutualInformationHistogram(256),
    // for itk::NormalizedMutualInformationHistogramImageToImageMetric
    m_NumberOfHistogramBinsNormalizedMutualInformationHistogram(256),
    // for itk::MattesMutualInformationImageToImageMetric
    m_NumberOfHistogramBinsMattesMutualInformation(256),
    m_UseSamplesMattesMutualInformation(true),
    m_SpatialSamplesMattesMutualInformation(500),
    // for itk::MeanReciprocalSquareDifferenceImageToImageMetric
    m_LambdaMeanReciprocalSquareDifference(10),
    // for itk::MutualInformationImageToImageMetric
    m_SpatialSamplesMutualInformation(100),
    m_FixedImageStandardDeviationMutualInformation(0.4),
    m_MovingImageStandardDeviationMutualInformation(0.4),
    m_UseNormalizerAndSmootherMutualInformation(true),
    m_FixedSmootherVarianceMutualInformation(0.2),
    m_MovingSmootherVarianceMutualInformation(0.2)
  {

  }
} // namespace mitk
