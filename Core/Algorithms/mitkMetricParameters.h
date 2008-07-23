/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkImageToImageFilter.h,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 11215 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKMETRICPARAMETERS_H
#define MITKMETRICPARAMETERS_H

#include <itkObjectFactory.h>
#include "mitkCommon.h"

namespace mitk {

  class MITK_CORE_EXPORT MetricParameters : public itk::Object
  {
  public:
    mitkClassMacro(MetricParameters,::itk::Object);
    itkNewMacro(Self);

    enum MetricType {
      MEANSQUARESIMAGETOIMAGEMETRIC = 0,
      NORMALIZEDCORRELATIONIMAGETOIMAGEMETRIC = 1,
      GRADIENTDIFFERENCEIMAGETOIMAGEMETRIC = 2,
      KULLBACKLEIBLERCOMPAREHISTOGRAMIMAGETOIMAGEMETRIC = 3,
      CORRELATIONCOEFFICIENTHISTOGRAMIMAGETOIMAGEMETRIC = 4,
      MEANSQUARESHISTOGRAMIMAGETOIMAGEMETRIC = 5,
      MUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC = 6,
      NORMALIZEDMUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC = 7,
      MATTESMUTUALINFORMATIONIMAGETOIMAGEMETRIC = 8,
      MEANRECIPROCALSQUAREDIFFERENCEIMAGETOIMAGEMETRIC = 9,
      MUTUALINFORMATIONIMAGETOIMAGEMETRIC = 10,
      MATCHCARDINALITYIMAGETOIMAGEMETRIC = 11,
      KAPPASTATISTICIMAGETOIMAGEMETRIC = 12
    };

    itkSetMacro( Metric, int );
    itkGetMacro( Metric, int );

    itkSetMacro( ComputeGradient, bool );
    itkGetMacro( ComputeGradient, bool );

    // for itk::KullbackLeiblerCompareHistogramImageToImageMetric
    itkSetMacro( NumberOfHistogramBinsKullbackLeiblerCompareHistogram, unsigned int );
    itkGetMacro( NumberOfHistogramBinsKullbackLeiblerCompareHistogram, unsigned int );
    // for itk::CorrelationCoefficientHistogramImageToImageMetric
    itkSetMacro( NumberOfHistogramBinsCorrelationCoefficientHistogram, unsigned int );
    itkGetMacro( NumberOfHistogramBinsCorrelationCoefficientHistogram, unsigned int );
    // for itk::MeanSquaresHistogramImageToImageMetric
    itkSetMacro( NumberOfHistogramBinsMeanSquaresHistogram, unsigned int );
    itkGetMacro( NumberOfHistogramBinsMeanSquaresHistogram, unsigned int );
    // for itk::MutualInformationHistogramImageToImageMetric
    itkSetMacro( NumberOfHistogramBinsMutualInformationHistogram, unsigned int );
    itkGetMacro( NumberOfHistogramBinsMutualInformationHistogram, unsigned int );
    // for itk::NormalizedMutualInformationHistogramImageToImageMetric
    itkSetMacro( NumberOfHistogramBinsNormalizedMutualInformationHistogram, unsigned int );
    itkGetMacro( NumberOfHistogramBinsNormalizedMutualInformationHistogram, unsigned int );
    // for itk::MattesMutualInformationImageToImageMetric
    itkSetMacro( SpatialSamplesMattesMutualInformation, unsigned int );
    itkGetMacro( SpatialSamplesMattesMutualInformation, unsigned int );
    itkSetMacro( UseSamplesMattesMutualInformation, bool );
    itkGetMacro( UseSamplesMattesMutualInformation, bool );
    itkSetMacro( NumberOfHistogramBinsMattesMutualInformation, unsigned int );
    itkGetMacro( NumberOfHistogramBinsMattesMutualInformation, unsigned int );
    // for itk::MeanReciprocalSquareDifferenceImageToImageMetric
    itkSetMacro( LambdaMeanReciprocalSquareDifference, unsigned int );
    itkGetMacro( LambdaMeanReciprocalSquareDifference, unsigned int );
    // for itk::MutualInformationImageToImageMetric
    itkSetMacro( SpatialSamplesMutualInformation, unsigned int );
    itkGetMacro( SpatialSamplesMutualInformation, unsigned int );
    itkSetMacro( FixedImageStandardDeviationMutualInformation, float );
    itkGetMacro( FixedImageStandardDeviationMutualInformation, float );
    itkSetMacro( MovingImageStandardDeviationMutualInformation, float );
    itkGetMacro( MovingImageStandardDeviationMutualInformation, float );
    itkSetMacro( UseNormalizerAndSmootherMutualInformation, bool );
    itkGetMacro( UseNormalizerAndSmootherMutualInformation, bool );
    itkSetMacro( FixedSmootherVarianceMutualInformation, float );
    itkGetMacro( FixedSmootherVarianceMutualInformation, float );
    itkSetMacro( MovingSmootherVarianceMutualInformation, float );
    itkGetMacro( MovingSmootherVarianceMutualInformation, float );

  protected:
    MetricParameters();
    ~MetricParameters() {};

    int m_Metric;
    bool m_ComputeGradient;
    // for itk::KullbackLeiblerCompareHistogramImageToImageMetric
    unsigned int m_NumberOfHistogramBinsKullbackLeiblerCompareHistogram;
    // for itk::CorrelationCoefficientHistogramImageToImageMetric
    unsigned int m_NumberOfHistogramBinsCorrelationCoefficientHistogram;
    // for itk::MeanSquaresHistogramImageToImageMetric
    unsigned int m_NumberOfHistogramBinsMeanSquaresHistogram;
    // for itk::MutualInformationHistogramImageToImageMetric
    unsigned int m_NumberOfHistogramBinsMutualInformationHistogram;
    // for itk::NormalizedMutualInformationHistogramImageToImageMetric
    unsigned int m_NumberOfHistogramBinsNormalizedMutualInformationHistogram;
    // for itk::MattesMutualInformationImageToImageMetric
    unsigned int m_NumberOfHistogramBinsMattesMutualInformation;
    bool m_UseSamplesMattesMutualInformation;
    unsigned int m_SpatialSamplesMattesMutualInformation;
    // for itk::MeanReciprocalSquareDifferenceImageToImageMetric
    unsigned int m_LambdaMeanReciprocalSquareDifference;
    // for itk::MutualInformationImageToImageMetric
    unsigned int m_SpatialSamplesMutualInformation;
    float m_FixedImageStandardDeviationMutualInformation;
    float m_MovingImageStandardDeviationMutualInformation;
    bool m_UseNormalizerAndSmootherMutualInformation;
    float m_FixedSmootherVarianceMutualInformation;
    float m_MovingSmootherVarianceMutualInformation;
  };

} // namespace mitk

#endif // MITKMETRICPARAMETERS_H
