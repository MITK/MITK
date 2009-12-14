/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: $
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkMetricFactory.h"
#include "mitkMetricParameters.h"

#include <itkMeanSquaresImageToImageMetric.h>
#include <itkMutualInformationImageToImageMetric.h>
#include <itkNormalizedCorrelationImageToImageMetric.h>
#include <itkMeanReciprocalSquareDifferenceImageToImageMetric.h>
#include <itkMattesMutualInformationImageToImageMetric.h>
#include <itkKullbackLeiblerCompareHistogramImageToImageMetric.h>
#include <itkGradientDifferenceImageToImageMetric.h>
#include <itkCorrelationCoefficientHistogramImageToImageMetric.h>
#include <itkMeanSquaresHistogramImageToImageMetric.h>
#include <itkMutualInformationHistogramImageToImageMetric.h>
#include <itkNormalizedMutualInformationHistogramImageToImageMetric.h>
#include <itkMatchCardinalityImageToImageMetric.h>
#include <itkKappaStatisticImageToImageMetric.h>

namespace mitk {

  template < class TPixelType, unsigned int VImageDimension >
  MetricFactory<TPixelType, VImageDimension>::MetricFactory() : m_MetricParameters(NULL)
  {
  }

  template < class TPixelType, unsigned int VImageDimension >
    typename MetricFactory< TPixelType, VImageDimension>::MetricPointer
    MetricFactory<TPixelType, VImageDimension>
    ::GetMetric( )
  {
    int metric = m_MetricParameters->GetMetric();
    if (metric == MetricParameters::NORMALIZEDCORRELATIONIMAGETOIMAGEMETRIC)
    {
      typename itk::NormalizedCorrelationImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::NormalizedCorrelationImageToImageMetric<FixedImageType, MovingImageType>::New();
      MetricPointer->SetComputeGradient(m_MetricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::GRADIENTDIFFERENCEIMAGETOIMAGEMETRIC)
    {
      typename itk::GradientDifferenceImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::GradientDifferenceImageToImageMetric<FixedImageType, MovingImageType>::New();
      MetricPointer->SetComputeGradient(m_MetricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::KULLBACKLEIBLERCOMPAREHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      typename itk::KullbackLeiblerCompareHistogramImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::KullbackLeiblerCompareHistogramImageToImageMetric<FixedImageType, MovingImageType>::New();
      unsigned int nBins = m_MetricParameters->GetNumberOfHistogramBinsKullbackLeiblerCompareHistogram();
      typename itk::KullbackLeiblerCompareHistogramImageToImageMetric<FixedImageType, MovingImageType>::HistogramType::SizeType histogramSize;
      histogramSize[0] = nBins;
      histogramSize[1] = nBins;
      MetricPointer->SetHistogramSize(histogramSize);
      MetricPointer->SetComputeGradient(m_MetricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::CORRELATIONCOEFFICIENTHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      typename itk::CorrelationCoefficientHistogramImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::CorrelationCoefficientHistogramImageToImageMetric<FixedImageType, MovingImageType>::New();
      unsigned int nBins = m_MetricParameters->GetNumberOfHistogramBinsCorrelationCoefficientHistogram();
      typename itk::CorrelationCoefficientHistogramImageToImageMetric<FixedImageType, MovingImageType>::HistogramType::SizeType histogramSize;
      histogramSize[0] = nBins;
      histogramSize[1] = nBins;
      MetricPointer->SetHistogramSize(histogramSize);
      MetricPointer->SetComputeGradient(m_MetricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::MUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      typename itk::MutualInformationHistogramImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::MutualInformationHistogramImageToImageMetric<FixedImageType, MovingImageType>::New();
      unsigned int nBins = m_MetricParameters->GetNumberOfHistogramBinsMutualInformationHistogram();
      typename itk::MutualInformationHistogramImageToImageMetric<FixedImageType, MovingImageType>::HistogramType::SizeType histogramSize;
      histogramSize[0] = nBins;
      histogramSize[1] = nBins;
      MetricPointer->SetHistogramSize(histogramSize);
      MetricPointer->SetComputeGradient(m_MetricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::NORMALIZEDMUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      typename itk::NormalizedMutualInformationHistogramImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::NormalizedMutualInformationHistogramImageToImageMetric<FixedImageType, MovingImageType>::New();
      unsigned int nBins = m_MetricParameters->GetNumberOfHistogramBinsNormalizedMutualInformationHistogram();
      typename itk::NormalizedMutualInformationHistogramImageToImageMetric<FixedImageType, MovingImageType>::HistogramType::SizeType histogramSize;
      histogramSize[0] = nBins;
      histogramSize[1] = nBins;
      MetricPointer->SetHistogramSize( histogramSize );
      MetricPointer->SetComputeGradient(m_MetricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::MATTESMUTUALINFORMATIONIMAGETOIMAGEMETRIC)
    {
      typename itk::MattesMutualInformationImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::MattesMutualInformationImageToImageMetric<FixedImageType, MovingImageType>::New();
      bool useSampling = m_MetricParameters->GetUseSamplesMattesMutualInformation();
      if( useSampling )
      {
        // set the number of samples to use
        MetricPointer->SetNumberOfSpatialSamples( m_MetricParameters->GetSpatialSamplesMattesMutualInformation() );
      }
      else
      {
        MetricPointer->UseAllPixelsOn();
      }
      MetricPointer->SetNumberOfHistogramBins(m_MetricParameters->GetNumberOfHistogramBinsMattesMutualInformation());
      MetricPointer->ReinitializeSeed( 76926294 );
      MetricPointer->SetComputeGradient(m_MetricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::MEANRECIPROCALSQUAREDIFFERENCEIMAGETOIMAGEMETRIC)
    {
      typename itk::MeanReciprocalSquareDifferenceImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::MeanReciprocalSquareDifferenceImageToImageMetric<FixedImageType, MovingImageType>::New();
      //------------------------------------------------------------
      // The lambda value is the intensity difference that should
      // make the metric drop by 50%
      //------------------------------------------------------------
      MetricPointer->SetLambda( m_MetricParameters->GetLambdaMeanReciprocalSquareDifference() );
      MetricPointer->SetComputeGradient(m_MetricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::MUTUALINFORMATIONIMAGETOIMAGEMETRIC)
    {
      typename itk::MutualInformationImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::MutualInformationImageToImageMetric<FixedImageType, MovingImageType>::New();
      MetricPointer->SetNumberOfSpatialSamples(m_MetricParameters->GetSpatialSamplesMutualInformation());
      MetricPointer->SetFixedImageStandardDeviation(m_MetricParameters->GetFixedImageStandardDeviationMutualInformation());
      MetricPointer->SetMovingImageStandardDeviation(m_MetricParameters->GetMovingImageStandardDeviationMutualInformation());
      MetricPointer->SetComputeGradient(m_MetricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::MATCHCARDINALITYIMAGETOIMAGEMETRIC)
    {
      typename itk::MatchCardinalityImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::MatchCardinalityImageToImageMetric<FixedImageType, MovingImageType>::New();
      MetricPointer->SetComputeGradient(m_MetricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::KAPPASTATISTICIMAGETOIMAGEMETRIC)
    {
      typename itk::KappaStatisticImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::KappaStatisticImageToImageMetric<FixedImageType, MovingImageType>::New();
      MetricPointer->SetComputeGradient(m_MetricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    return NULL;
  }
} // end namespace
