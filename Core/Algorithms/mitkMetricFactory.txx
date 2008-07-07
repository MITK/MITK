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
  MetricFactory<TPixelType, VImageDimension>::MetricFactory()
  {
  }

  template < class TPixelType, unsigned int VImageDimension > 
  MetricFactory<TPixelType, VImageDimension>::~MetricFactory()
  {
  }

  template < class TPixelType, unsigned int VImageDimension >
    typename MetricFactory< TPixelType, VImageDimension>::MetricPointer
    MetricFactory<TPixelType, VImageDimension>
    ::GetMetric( )
  {
    MetricParameters* metricParameters = MetricParameters::GetInstance();
    int metric = metricParameters->GetMetric();
    if (metric == MetricParameters::MEANSQUARESIMAGETOIMAGEMETRIC)
    {
      typename itk::MeanSquaresImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::MeanSquaresImageToImageMetric<FixedImageType, MovingImageType>::New();
      MetricPointer->SetComputeGradient(metricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::NORMALIZEDCORRELATIONIMAGETOIMAGEMETRIC)
    {
      typename itk::NormalizedCorrelationImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::NormalizedCorrelationImageToImageMetric<FixedImageType, MovingImageType>::New();
      MetricPointer->SetComputeGradient(metricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::GRADIENTDIFFERENCEIMAGETOIMAGEMETRIC)
    {
      typename itk::GradientDifferenceImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::GradientDifferenceImageToImageMetric<FixedImageType, MovingImageType>::New();
      MetricPointer->SetComputeGradient(metricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::KULLBACKLEIBLERCOMPAREHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      typename itk::KullbackLeiblerCompareHistogramImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::KullbackLeiblerCompareHistogramImageToImageMetric<FixedImageType, MovingImageType>::New();
      unsigned int nBins = metricParameters->GetNumberOfHistogramBinsKullbackLeiblerCompareHistogram();
      typename itk::KullbackLeiblerCompareHistogramImageToImageMetric<FixedImageType, MovingImageType>::HistogramType::SizeType histogramSize;
      if (VImageDimension == 2)
      {
        histogramSize[0] = nBins;
        histogramSize[1] = nBins;
      }
      else if(VImageDimension == 3)
      {
        histogramSize[0] = nBins;
        histogramSize[1] = nBins;
        histogramSize[2] = nBins;
      }
      MetricPointer->SetHistogramSize(histogramSize);
      MetricPointer->SetComputeGradient(metricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::CORRELATIONCOEFFICIENTHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      typename itk::CorrelationCoefficientHistogramImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::CorrelationCoefficientHistogramImageToImageMetric<FixedImageType, MovingImageType>::New();
      unsigned int nBins = metricParameters->GetNumberOfHistogramBinsCorrelationCoefficientHistogram();
      typename itk::CorrelationCoefficientHistogramImageToImageMetric<FixedImageType, MovingImageType>::HistogramType::SizeType histogramSize;
      if (VImageDimension == 2)
      {
        histogramSize[0] = nBins;
        histogramSize[1] = nBins;
      }
      else if(VImageDimension == 3)
      {
        histogramSize[0] = nBins;
        histogramSize[1] = nBins;
        histogramSize[2] = nBins;
      }
      MetricPointer->SetHistogramSize(histogramSize);
      MetricPointer->SetComputeGradient(metricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::MEANSQUARESHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      typename itk::MeanSquaresHistogramImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::MeanSquaresHistogramImageToImageMetric<FixedImageType, MovingImageType>::New();
      unsigned int nBins = metricParameters->GetNumberOfHistogramBinsMeanSquaresHistogram();
      typename itk::MeanSquaresHistogramImageToImageMetric<FixedImageType, MovingImageType>::HistogramType::SizeType histogramSize;
      if (VImageDimension == 2)
      {
        histogramSize[0] = nBins;
        histogramSize[1] = nBins;
      }
      else if(VImageDimension == 3)
      {
        histogramSize[0] = nBins;
        histogramSize[1] = nBins;
        histogramSize[2] = nBins;
      }
      MetricPointer->SetHistogramSize(histogramSize);
      MetricPointer->SetComputeGradient(metricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::MUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      typename itk::MutualInformationHistogramImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::MutualInformationHistogramImageToImageMetric<FixedImageType, MovingImageType>::New();
      unsigned int nBins = metricParameters->GetNumberOfHistogramBinsMutualInformationHistogram();
      typename itk::MutualInformationHistogramImageToImageMetric<FixedImageType, MovingImageType>::HistogramType::SizeType histogramSize;
      if (VImageDimension == 2)
      {
        histogramSize[0] = nBins;
        histogramSize[1] = nBins;
      }
      else if(VImageDimension == 3)
      {
        histogramSize[0] = nBins;
        histogramSize[1] = nBins;
        histogramSize[2] = nBins;
      }
      MetricPointer->SetHistogramSize(histogramSize);
      MetricPointer->SetComputeGradient(metricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::NORMALIZEDMUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      typename itk::NormalizedMutualInformationHistogramImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::NormalizedMutualInformationHistogramImageToImageMetric<FixedImageType, MovingImageType>::New();
      unsigned int nBins = metricParameters->GetNumberOfHistogramBinsNormalizedMutualInformationHistogram();
      typename itk::NormalizedMutualInformationHistogramImageToImageMetric<FixedImageType, MovingImageType>::HistogramType::SizeType histogramSize;
      if (VImageDimension == 2)
      {
        histogramSize[0] = nBins;
        histogramSize[1] = nBins;
      }
      else if(VImageDimension == 3)
      {
        histogramSize[0] = nBins;
        histogramSize[1] = nBins;
        histogramSize[2] = nBins;
      }
      MetricPointer->SetHistogramSize( histogramSize );
      MetricPointer->SetComputeGradient(metricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::MATTESMUTUALINFORMATIONIMAGETOIMAGEMETRIC)
    {
      typename itk::MattesMutualInformationImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::MattesMutualInformationImageToImageMetric<FixedImageType, MovingImageType>::New();
      bool useSampling = metricParameters->GetUseSamplesMattesMutualInformation();
      if( useSampling )
      {
        // set the number of samples to use
        MetricPointer->SetNumberOfSpatialSamples( metricParameters->GetSpatialSamplesMattesMutualInformation() );
      }
      else
      {
        MetricPointer->UseAllPixelsOn();
      }
      MetricPointer->SetNumberOfHistogramBins(metricParameters->GetNumberOfHistogramBinsMattesMutualInformation());
      MetricPointer->ReinitializeSeed( 76926294 );
      MetricPointer->SetComputeGradient(metricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::MEANRECIPROCALSQUAREDIFFERENCEIMAGETOIMAGEMETRIC)
    {
      typename itk::MeanReciprocalSquareDifferenceImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::MeanReciprocalSquareDifferenceImageToImageMetric<FixedImageType, MovingImageType>::New();
      //------------------------------------------------------------
      // The lambda value is the intensity difference that should
      // make the metric drop by 50%
      //------------------------------------------------------------
      MetricPointer->SetLambda( metricParameters->GetLambdaMeanReciprocalSquareDifference() );
      MetricPointer->SetComputeGradient(metricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::MUTUALINFORMATIONIMAGETOIMAGEMETRIC)
    {
      typename itk::MutualInformationImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::MutualInformationImageToImageMetric<FixedImageType, MovingImageType>::New();
      MetricPointer->SetNumberOfSpatialSamples(metricParameters->GetSpatialSamplesMutualInformation());
      MetricPointer->SetFixedImageStandardDeviation(metricParameters->GetFixedImageStandardDeviationMutualInformation());
      MetricPointer->SetMovingImageStandardDeviation(metricParameters->GetMovingImageStandardDeviationMutualInformation());
      MetricPointer->SetComputeGradient(metricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::MATCHCARDINALITYIMAGETOIMAGEMETRIC)
    {
      typename itk::MatchCardinalityImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::MatchCardinalityImageToImageMetric<FixedImageType, MovingImageType>::New();
      MetricPointer->SetComputeGradient(metricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    else if (metric == MetricParameters::KAPPASTATISTICIMAGETOIMAGEMETRIC)
    {
      typename itk::KappaStatisticImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::KappaStatisticImageToImageMetric<FixedImageType, MovingImageType>::New();
      MetricPointer->SetComputeGradient(metricParameters->GetComputeGradient());
      return MetricPointer.GetPointer();
    }
    return NULL;
  }
} // end namespace
