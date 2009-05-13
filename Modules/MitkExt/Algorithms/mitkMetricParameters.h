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

  /*!
  \brief This class is used to hold all metric parameters needed for a rigid registration process.  

  To use the rigid registration framework you have to create an instance of this class and fill it with the parameters
  belonging to the selected metric. To let the rigid registration work properly, this instance has to be given 
  to mitkImageRegistrationMethod before calling the update() method in mitkImageRegistrationMethod.

  Also instances of the classes mitkTransformParameters and mitkOptimizerParameters have to be set in mitkImageRegistrationMethod 
  before calling the update() method.


  \ingroup RigidRegistration

  \author Daniel Stein
  */
  class MITKEXT_CORE_EXPORT MetricParameters : public itk::Object
  {
  public:
    mitkClassMacro(MetricParameters,::itk::Object);
    itkNewMacro(Self);

    /**
    \brief Unique integer value for every metric.
    */
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

    /**
    \brief Sets the metric used for registration by its unique integer value.
    */
    itkSetMacro( Metric, int );
    /**
    \brief Returns the metric used for registration by its unique integer value.
    */
    itkGetMacro( Metric, int );

    /**
    \brief Sets whether a gradient image has to be computed. Some optimizer need this.
    */
    itkSetMacro( ComputeGradient, bool );
    /**
    \brief Returns whether a gradient image has to be computed.
    */
    itkGetMacro( ComputeGradient, bool );

    /**
    \brief for itk::KullbackLeiblerCompareHistogramImageToImageMetric
    */
    itkSetMacro( NumberOfHistogramBinsKullbackLeiblerCompareHistogram, unsigned int );

    /**
    \brief for itk::KullbackLeiblerCompareHistogramImageToImageMetric
    */
    itkGetMacro( NumberOfHistogramBinsKullbackLeiblerCompareHistogram, unsigned int );

    /**
    \brief for itk::CorrelationCoefficientHistogramImageToImageMetric
    */
    itkSetMacro( NumberOfHistogramBinsCorrelationCoefficientHistogram, unsigned int );

    /**
    \brief for itk::CorrelationCoefficientHistogramImageToImageMetric
    */
    itkGetMacro( NumberOfHistogramBinsCorrelationCoefficientHistogram, unsigned int );

    /**
    \brief for itk::MeanSquaresHistogramImageToImageMetric
    */
    itkSetMacro( NumberOfHistogramBinsMeanSquaresHistogram, unsigned int );

    /**
    \brief for itk::MeanSquaresHistogramImageToImageMetric
    */
    itkGetMacro( NumberOfHistogramBinsMeanSquaresHistogram, unsigned int );

    /**
    \brief for itk::MutualInformationHistogramImageToImageMetric
    */
    itkSetMacro( NumberOfHistogramBinsMutualInformationHistogram, unsigned int );

    /**
    \brief for itk::MutualInformationHistogramImageToImageMetric
    */
    itkGetMacro( NumberOfHistogramBinsMutualInformationHistogram, unsigned int );

    /**
    \brief for itk::NormalizedMutualInformationHistogramImageToImageMetric
    */
    itkSetMacro( NumberOfHistogramBinsNormalizedMutualInformationHistogram, unsigned int );

    /**
    \brief for itk::NormalizedMutualInformationHistogramImageToImageMetric
    */
    itkGetMacro( NumberOfHistogramBinsNormalizedMutualInformationHistogram, unsigned int );

    /**
    \brief for itk::MattesMutualInformationImageToImageMetric
    */
    itkSetMacro( SpatialSamplesMattesMutualInformation, unsigned int );

    /**
    \brief for itk::MattesMutualInformationImageToImageMetric
    */
    itkGetMacro( SpatialSamplesMattesMutualInformation, unsigned int );

    /**
    \brief for itk::MattesMutualInformationImageToImageMetric
    */
    itkSetMacro( UseSamplesMattesMutualInformation, bool );

    /**
    \brief for itk::MattesMutualInformationImageToImageMetric
    */
    itkGetMacro( UseSamplesMattesMutualInformation, bool );

    /**
    \brief for itk::MattesMutualInformationImageToImageMetric
    */
    itkSetMacro( NumberOfHistogramBinsMattesMutualInformation, unsigned int );

    /**
    \brief for itk::MattesMutualInformationImageToImageMetric
    */
    itkGetMacro( NumberOfHistogramBinsMattesMutualInformation, unsigned int );

    /**
    \brief for itk::MeanReciprocalSquareDifferenceImageToImageMetric
    */
    itkSetMacro( LambdaMeanReciprocalSquareDifference, unsigned int );

    /**
    \brief for itk::MeanReciprocalSquareDifferenceImageToImageMetric
    */
    itkGetMacro( LambdaMeanReciprocalSquareDifference, unsigned int );

    /**
    \brief for itk::MutualInformationImageToImageMetric
    */
    itkSetMacro( SpatialSamplesMutualInformation, unsigned int );

    /**
    \brief for itk::MutualInformationImageToImageMetric
    */
    itkGetMacro( SpatialSamplesMutualInformation, unsigned int );

    /**
    \brief for itk::MutualInformationImageToImageMetric
    */
    itkSetMacro( FixedImageStandardDeviationMutualInformation, float );

    /**
    \brief for itk::MutualInformationImageToImageMetric
    */
    itkGetMacro( FixedImageStandardDeviationMutualInformation, float );

    /**
    \brief for itk::MutualInformationImageToImageMetric
    */
    itkSetMacro( MovingImageStandardDeviationMutualInformation, float );

    /**
    \brief for itk::MutualInformationImageToImageMetric
    */
    itkGetMacro( MovingImageStandardDeviationMutualInformation, float );

    /**
    \brief for itk::MutualInformationImageToImageMetric
    */
    itkSetMacro( UseNormalizerAndSmootherMutualInformation, bool );

    /**
    \brief for itk::MutualInformationImageToImageMetric
    */
    itkGetMacro( UseNormalizerAndSmootherMutualInformation, bool );

    /**
    \brief for itk::MutualInformationImageToImageMetric
    */
    itkSetMacro( FixedSmootherVarianceMutualInformation, float );

    /**
    \brief for itk::MutualInformationImageToImageMetric
    */
    itkGetMacro( FixedSmootherVarianceMutualInformation, float );

    /**
    \brief for itk::MutualInformationImageToImageMetric
    */
    itkSetMacro( MovingSmootherVarianceMutualInformation, float );

    /**
    \brief for itk::MutualInformationImageToImageMetric
    */
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
