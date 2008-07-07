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

#include <itkObject.h>
#include "mitkCommon.h"

namespace mitk {

  class MITK_CORE_EXPORT MetricParameters : public itk::Object
  {
  public:
    itkTypeMacro(MetricParameters, itk::Object);

    static MetricParameters* GetInstance(); //singleton

    static const int MEANSQUARESIMAGETOIMAGEMETRIC = 0;
    static const int NORMALIZEDCORRELATIONIMAGETOIMAGEMETRIC = 1;
    static const int GRADIENTDIFFERENCEIMAGETOIMAGEMETRIC = 2;
    static const int KULLBACKLEIBLERCOMPAREHISTOGRAMIMAGETOIMAGEMETRIC = 3;
    static const int CORRELATIONCOEFFICIENTHISTOGRAMIMAGETOIMAGEMETRIC = 4;
    static const int MEANSQUARESHISTOGRAMIMAGETOIMAGEMETRIC = 5;
    static const int MUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC = 6;
    static const int NORMALIZEDMUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC = 7;
    static const int MATTESMUTUALINFORMATIONIMAGETOIMAGEMETRIC = 8;
    static const int MEANRECIPROCALSQUAREDIFFERENCEIMAGETOIMAGEMETRIC = 9;
    static const int MUTUALINFORMATIONIMAGETOIMAGEMETRIC = 10;
    static const int MATCHCARDINALITYIMAGETOIMAGEMETRIC = 11;
    static const int KAPPASTATISTICIMAGETOIMAGEMETRIC = 12;

    void SetMetric(int metric);
    int GetMetric();

    void SetComputeGradient(bool gradient);
    bool GetComputeGradient();

    // for itk::KullbackLeiblerCompareHistogramImageToImageMetric
    void SetNumberOfHistogramBinsKullbackLeiblerCompareHistogram(unsigned int bins);
    unsigned int GetNumberOfHistogramBinsKullbackLeiblerCompareHistogram();
    // for itk::CorrelationCoefficientHistogramImageToImageMetric
    void SetNumberOfHistogramBinsCorrelationCoefficientHistogram(unsigned int bins);
    unsigned int GetNumberOfHistogramBinsCorrelationCoefficientHistogram();
    // for itk::MeanSquaresHistogramImageToImageMetric
    void SetNumberOfHistogramBinsMeanSquaresHistogram(unsigned int bins);
    unsigned int GetNumberOfHistogramBinsMeanSquaresHistogram();
    // for itk::MutualInformationHistogramImageToImageMetric
    void SetNumberOfHistogramBinsMutualInformationHistogram(unsigned int bins);
    unsigned int GetNumberOfHistogramBinsMutualInformationHistogram();
    // for itk::NormalizedMutualInformationHistogramImageToImageMetric
    void SetNumberOfHistogramBinsNormalizedMutualInformationHistogram(unsigned int bins);
    unsigned int GetNumberOfHistogramBinsNormalizedMutualInformationHistogram();
    // for itk::MattesMutualInformationImageToImageMetric
    void SetSpatialSamplesMattesMutualInformation(unsigned int spatialSamples);
    unsigned int GetSpatialSamplesMattesMutualInformation();
    void SetUseSamplesMattesMutualInformation(bool samples);
    bool GetUseSamplesMattesMutualInformation();
    void SetNumberOfHistogramBinsMattesMutualInformation(unsigned int bins);
    unsigned int GetNumberOfHistogramBinsMattesMutualInformation();
    // for itk::MeanReciprocalSquareDifferenceImageToImageMetric
    void SetLambdaMeanReciprocalSquareDifference(unsigned int lambda);
    unsigned int GetLambdaMeanReciprocalSquareDifference();
    // for itk::MutualInformationImageToImageMetric
    void SetSpatialSamplesMutualInformation(unsigned int spatialSamples);
    unsigned int GetSpatialSamplesMutualInformation();
    void SetFixedImageStandardDeviationMutualInformation(float fixedStandardDev);
    float GetFixedImageStandardDeviationMutualInformation();
    void SetMovingImageStandardDeviationMutualInformation(float movingStandardDev);
    float GetMovingImageStandardDeviationMutualInformation();
    void SetUseNormalizerAndSmootherMutualInformation(bool useNormalizer);
    bool GetUseNormalizerAndSmootherMutualInformation();
    void SetFixedSmootherVarianceMutualInformation(float fixedSmootherVariance);
    float GetFixedSmootherVarianceMutualInformation();
    void SetMovingSmootherVarianceMutualInformation(float movingSmootherVariance);
    float GetMovingSmootherVarianceMutualInformation();

  protected:
    MetricParameters(); // hidden, access through GetInstance()
    ~MetricParameters();
    static MetricParameters* m_Instance;
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
    bool m_UseSamplingMattesMutualInformation;
    unsigned int m_NumberOfSpatialSamplesMattesMutualInformation;
    // for itk::MeanReciprocalSquareDifferenceImageToImageMetric
    unsigned int m_LambdaMeanReciprocalSquareDifference;
    // for itk::MutualInformationImageToImageMetric
    unsigned int m_NumberOfSpatialSamplesMutualInformation;
    float m_FixedImageStandardDeviationMutualInformation;
    float m_MovingImageStandardDeviationMutualInformation;
    bool m_UseNormalizer;
    float m_FixedSmootherVariance;
    float m_MovingSmootherVariance;
  };

} // namespace mitk

#endif // MITKMETRICPARAMETERS_H
