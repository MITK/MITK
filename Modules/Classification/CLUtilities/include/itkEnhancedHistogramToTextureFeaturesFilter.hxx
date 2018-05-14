/*=========================================================================
*
*  Copyright Insight Software Consortium
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*         http://www.apache.org/licenses/LICENSE-2.0.txt
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*=========================================================================*/
#ifndef __itkEnhancedHistogramToTextureFeaturesFilter_hxx
#define __itkEnhancedHistogramToTextureFeaturesFilter_hxx

#include "itkEnhancedHistogramToTextureFeaturesFilter.h"

#include "itkNumericTraits.h"
#include "vnl/vnl_math.h"
#include "itkMath.h"

#define itkMacroGLCMFeature(name, id)                                       \
  template< typename THistogram >                                                                   \
  const                                                                                             \
  typename EnhancedHistogramToTextureFeaturesFilter< THistogram >::MeasurementObjectType *          \
  EnhancedHistogramToTextureFeaturesFilter< THistogram >                                            \
  ::Get##name##Output() const                                                                         \
{                                                                                                   \
  return static_cast< const MeasurementObjectType * >( this->ProcessObject::GetOutput(id) );        \
  }                                                                                                   \
  \
  template< typename THistogram >                                                                   \
  typename EnhancedHistogramToTextureFeaturesFilter< THistogram >::MeasurementType                  \
  EnhancedHistogramToTextureFeaturesFilter< THistogram >                                            \
  ::Get##name() const                                                                               \
{                                                                                                   \
  return this->Get##name##Output()->Get();                                                            \
  }

namespace itk
{
namespace Statistics
{
//constructor
template< typename THistogram >
EnhancedHistogramToTextureFeaturesFilter< THistogram >::EnhancedHistogramToTextureFeaturesFilter(void)
{
  this->ProcessObject::SetNumberOfRequiredInputs(1);

  // allocate the data objects for the outputs which are
  // just decorators real types
  for ( int i = 0; i < 28; ++i )
  {
    this->ProcessObject::SetNthOutput( i, this->MakeOutput(i) );
  }
}

template< typename THistogram >
void
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::SetInput(const HistogramType *histogram)
{
  this->ProcessObject::SetNthInput( 0, const_cast< HistogramType * >( histogram ) );
}

template< typename THistogram >
const typename
EnhancedHistogramToTextureFeaturesFilter< THistogram >::HistogramType *
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::GetInput() const
{
  return itkDynamicCastInDebugMode< const HistogramType * >( this->GetPrimaryInput() );
}

template< typename THistogram >
typename
EnhancedHistogramToTextureFeaturesFilter< THistogram >::DataObjectPointer
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::MakeOutput( DataObjectPointerArraySizeType itkNotUsed(idx) )
{
  return MeasurementObjectType::New().GetPointer();
}

template< typename THistogram >
void
EnhancedHistogramToTextureFeaturesFilter< THistogram >::GenerateData(void)
{
  typedef typename HistogramType::ConstIterator HistogramIterator;

  const HistogramType *inputHistogram = this->GetInput();

  //Normalize the absolute frequencies and populate the relative frequency
  //container
  TotalRelativeFrequencyType totalFrequency =
      static_cast< TotalRelativeFrequencyType >( inputHistogram->GetTotalFrequency() );

  m_RelativeFrequencyContainer.clear();

  typename HistogramType::SizeValueType binsPerAxis = inputHistogram->GetSize(0);
  std::vector<double> sumP;
  std::vector<double> diffP;

  sumP.resize(2*binsPerAxis,0.0);
  diffP.resize(binsPerAxis,0.0);

  double numberOfPixels = 0;

  for ( HistogramIterator hit = inputHistogram->Begin();
        hit != inputHistogram->End(); ++hit )
  {
    AbsoluteFrequencyType frequency = hit.GetFrequency();
    RelativeFrequencyType relativeFrequency =  (totalFrequency > 0) ? frequency / totalFrequency : 0;
    m_RelativeFrequencyContainer.push_back(relativeFrequency);

    IndexType index = inputHistogram->GetIndex( hit.GetInstanceIdentifier() );
    sumP[index[0] + index[1]] += relativeFrequency;
    diffP[std::abs(index[0] - index[1])] += relativeFrequency;

    //if (index[1] == 0)
    numberOfPixels += frequency;
  }

  // Now get the various means and variances. This is takes two passes
  // through the histogram.
  double pixelMean;
  double marginalMean;
  double marginalDevSquared;
  double pixelVariance;

  this->ComputeMeansAndVariances(pixelMean, marginalMean, marginalDevSquared,
                                 pixelVariance);

  // Finally compute the texture features. Another one pass.
  MeasurementType energy      = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType entropy     = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType correlation = NumericTraits< MeasurementType >::ZeroValue();

  MeasurementType inverseDifferenceMoment      =
      NumericTraits< MeasurementType >::ZeroValue();

  MeasurementType inertia             = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType clusterShade        = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType clusterProminence   = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType haralickCorrelation = NumericTraits< MeasurementType >::ZeroValue();

  MeasurementType autocorrelation = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType contrast = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType dissimilarity = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType maximumProbability = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType inverseVariance = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType homogeneity1 = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType clusterTendency = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType variance = NumericTraits< MeasurementType >::ZeroValue();

  MeasurementType sumAverage = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType sumEntropy = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType sumVariance = NumericTraits< MeasurementType >::ZeroValue();

  MeasurementType diffAverage = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType diffEntropy = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType diffVariance = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType inverseDifferenceMomentNormalized = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType inverseDifferenceNormalized      = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType inverseDifference      = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType averageProbability = NumericTraits< MeasurementType >::ZeroValue();

  MeasurementType firstMeasureOfInformationCorrelation = NumericTraits< MeasurementType >::ZeroValue();
  MeasurementType secondMeasureOfInformationCorrelation = NumericTraits< MeasurementType >::ZeroValue();

  double pixelVarianceSquared = pixelVariance * pixelVariance;
  // Variance is only used in correlation. If variance is 0, then
  //   (index[0] - pixelMean) * (index[1] - pixelMean)
  // should be zero as well. In this case, set the variance to 1. in
  // order to avoid NaN correlation.
  if( Math::FloatAlmostEqual( pixelVarianceSquared, 0.0, 4, 2*NumericTraits<double>::epsilon() ) )
  {
    pixelVarianceSquared = 1.;
  }
  const double log2 = std::log(2.0);

  typename RelativeFrequencyContainerType::const_iterator rFreqIterator =
      m_RelativeFrequencyContainer.begin();

  IndexType globalIndex(2);

  for ( HistogramIterator hit = inputHistogram->Begin();
        hit != inputHistogram->End(); ++hit )
  {
    RelativeFrequencyType frequency = *rFreqIterator;
    ++rFreqIterator;
    if ( frequency == 0 )
    {
      continue; // no use doing these calculations if we're just multiplying by
      // zero.
    }

    IndexType index = inputHistogram->GetIndex( hit.GetInstanceIdentifier() );
    globalIndex = index;
    energy += frequency * frequency;
    entropy -= ( frequency > 0.0001 ) ? frequency *std::log(frequency) / log2:0;
    correlation += ( ( index[0] - pixelMean ) * ( index[1] - pixelMean ) * frequency )
        / pixelVarianceSquared;
    inverseDifferenceMoment += frequency
        / ( 1.0 + ( index[0] - index[1] ) * ( index[0] - index[1] ) );
    inertia += ( index[0] - index[1] ) * ( index[0] - index[1] ) * frequency;
    clusterShade += std::pow( ( index[0] - pixelMean ) + ( index[1] - pixelMean ), 3 )
        * frequency;
    clusterProminence += std::pow( ( index[0] - pixelMean ) + ( index[1] - pixelMean ), 4 )
        * frequency;
    haralickCorrelation += index[0] * index[1] * frequency;

    // New Features added for Aerts compatibility
    autocorrelation +=index[0] * index[1] * frequency;
    contrast += (index[0] - index[1]) * (index[0] - index[1]) * frequency;
    dissimilarity += std::abs(index[0] - index[1]) * frequency;
    maximumProbability +=std::max(maximumProbability, frequency);
    averageProbability += frequency * index[0];
    if (index[0] != index[1])
      inverseVariance += frequency / ((index[0] - index[1])*(index[0] - index[1]));
    homogeneity1 +=frequency / ( 1.0 + std::abs( index[0] - index[1] ));
    clusterTendency += std::pow( ( index[0] - pixelMean ) + ( index[1] - pixelMean ), 2 ) * frequency;
    variance += std::pow( ( index[0] - pixelMean ), 2) * frequency;

    if (numberOfPixels > 0)
    {
      inverseDifferenceMomentNormalized += frequency / ( 1.0 + ( index[0] - index[1] ) * ( index[0] - index[1] ) / numberOfPixels / numberOfPixels);
      inverseDifferenceNormalized += frequency / ( 1.0 + std::abs( index[0] - index[1] ) / numberOfPixels );
    }
    else
    {
      inverseDifferenceMomentNormalized = 0;
      inverseDifferenceNormalized = 0;
    }
    inverseDifference += frequency / ( 1.0 + std::abs( index[0] - index[1] ) );
  }

  for (int i = 0; i < (int)sumP.size();++i)
  {
    double frequency = sumP[i];
    sumAverage += i * frequency;
    sumEntropy -= ( frequency > 0.0001 ) ? frequency *std::log(frequency) / log2:0;
  }
  for (int i = 0; i < (int)sumP.size();++i)
  {
    double frequency = sumP[i];
    sumVariance += (i-sumAverage)*(i-sumAverage) * frequency;
  }

  for (int i = 0; i < (int)diffP.size();++i)
  {
    double frequency = diffP[i];
    diffAverage += i * frequency;
    diffEntropy -= ( frequency > 0.0001 ) ? frequency *std::log(frequency) / log2:0;
  }
  for (int i = 0; i < (int)diffP.size();++i)
  {
    double frequency = diffP[i];
    sumVariance += (i-diffAverage)*(i-diffAverage) * frequency;
  }

  if (marginalDevSquared > 0)
  {
    haralickCorrelation = ( haralickCorrelation - marginalMean * marginalMean )
        / marginalDevSquared;
  } else
  {
    haralickCorrelation =0;
  }

  //Calculate the margin probs
  std::vector<double> pi_margins;
  std::vector<double> pj_margins;

  //pi.
  for ( std::size_t i = 1; i <= inputHistogram->GetSize(0); i++ )
  {
    double pi_tmp= 0.0;
    for( std::size_t j = 1; j <= inputHistogram->GetSize(1); j++ )
    {
      globalIndex[0] = i;
      globalIndex[1] = j;
      pi_tmp += inputHistogram->GetFrequency(globalIndex);
    }
    pi_margins.push_back(pi_tmp);
  }

  //pj.
  for ( std::size_t j = 1; j <= inputHistogram->GetSize(1); j++ )
  {
    double pj_tmp= 0.0;
    for( std::size_t i = 1; i <= inputHistogram->GetSize(0); i++ )
    {
      globalIndex[0] = i;
      globalIndex[1] = j;
      pj_tmp += inputHistogram->GetFrequency(globalIndex);
    }
    pj_margins.push_back(pj_tmp);
  }

  //Calculate HX
  double hx = 0.0;

  for ( std::size_t i = 0; i < inputHistogram->GetSize(0); i++ )
  {
    double pi_margin = pi_margins[i];
    hx -= ( pi_margin > 0.0001 ) ? pi_margin *std::log(pi_margin) / log2:0;
  }

  //Calculate HXY1
  double hxy1 = 0.0;

  for ( std::size_t i = 0; i < inputHistogram->GetSize(0); i++ )
  {
    for ( std::size_t j = 0; j < inputHistogram->GetSize(1); j++ )
    {
      globalIndex[0] = i;
      globalIndex[1] = j;

      double pi_margin = pi_margins[i];
      double pj_margin = pj_margins[j];

      double p_ij = inputHistogram->GetFrequency(globalIndex);

      hxy1 -= ( (pi_margin * pj_margin) > 0.0001 ) ? p_ij *std::log(pi_margin * pj_margin) / log2:0;
    }
  }

  //Calculate HXY2
  double hxy2 = 0.0;

  for ( std::size_t i = 0; i < inputHistogram->GetSize(0); i++ )
  {
    for ( std::size_t j = 0; j < inputHistogram->GetSize(1); j++ )
    {
      globalIndex[0] = i;
      globalIndex[1] = j;

      double pi_margin = pi_margins[i];
      double pj_margin = pj_margins[j];

      hxy1 -= ( (pi_margin * pj_margin) > 0.0001 ) ? (pi_margin * pj_margin) *std::log(pi_margin * pj_margin) / log2:0;
    }
  }

  firstMeasureOfInformationCorrelation = (entropy - hxy1) / hx;
  secondMeasureOfInformationCorrelation = (entropy > hxy2) ? 0 : std::sqrt(1 - std::exp(-2*(hxy2 - entropy)));

  MeasurementObjectType *energyOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(0) );
  energyOutputObject->Set(energy);

  MeasurementObjectType *entropyOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(1) );
  entropyOutputObject->Set(entropy);

  MeasurementObjectType *correlationOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(2) );
  correlationOutputObject->Set(correlation);

  MeasurementObjectType *inverseDifferenceMomentOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(3) );
  inverseDifferenceMomentOutputObject->Set(inverseDifferenceMoment);

  MeasurementObjectType *inertiaOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(4) );
  inertiaOutputObject->Set(inertia);

  MeasurementObjectType *clusterShadeOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(5) );
  clusterShadeOutputObject->Set(clusterShade);

  MeasurementObjectType *clusterProminenceOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(6) );
  clusterProminenceOutputObject->Set(clusterProminence);

  MeasurementObjectType *haralickCorrelationOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(7) );
  haralickCorrelationOutputObject->Set(haralickCorrelation);

  MeasurementObjectType *autocorrelationOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(8) );
  autocorrelationOutputObject->Set(autocorrelation);

  MeasurementObjectType *contrastOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(9) );
  contrastOutputObject->Set(contrast);

  MeasurementObjectType *dissimilarityOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(10) );
  dissimilarityOutputObject->Set(dissimilarity);

  MeasurementObjectType *maximumProbabilityOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(11) );
  maximumProbabilityOutputObject->Set(maximumProbability);

  MeasurementObjectType *inverseVarianceOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(12) );
  inverseVarianceOutputObject->Set(inverseVariance);

  MeasurementObjectType *homogeneity1OutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(13) );
  homogeneity1OutputObject->Set(homogeneity1);

  MeasurementObjectType *clusterTendencyOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(14) );
  clusterTendencyOutputObject->Set(clusterTendency);

  MeasurementObjectType *varianceOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(15) );
  varianceOutputObject->Set(variance);

  MeasurementObjectType *sumAverageOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(16) );
  sumAverageOutputObject->Set(sumAverage);

  MeasurementObjectType *sumEntropyOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(17) );
  sumEntropyOutputObject->Set(sumEntropy);

  MeasurementObjectType *sumVarianceOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(18) );
  sumVarianceOutputObject->Set(sumVariance);

  MeasurementObjectType *diffAverageOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(19) );
  diffAverageOutputObject->Set(diffAverage);

  MeasurementObjectType *diffEntropyOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(20) );
  diffEntropyOutputObject->Set(diffEntropy);

  MeasurementObjectType *diffVarianceOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(21) );
  diffVarianceOutputObject->Set(diffVariance);

  MeasurementObjectType *inverseDifferenceMomentNormalizedOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(22) );
  inverseDifferenceMomentNormalizedOutputObject->Set(inverseDifferenceMomentNormalized);

  MeasurementObjectType *inverseDifferenceNormalizedOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(23) );
  inverseDifferenceNormalizedOutputObject->Set(inverseDifferenceNormalized);

  MeasurementObjectType *inverseDifferenceOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(24) );
  inverseDifferenceOutputObject->Set(inverseDifference);

  MeasurementObjectType *jointAverageOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(25) );
  jointAverageOutputObject->Set(averageProbability);

  MeasurementObjectType *firstMeasureOfInformationCorrelationOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(26) );
  firstMeasureOfInformationCorrelationOutputObject->Set(firstMeasureOfInformationCorrelation);

  MeasurementObjectType *secondMeasureOfInformationCorrelationOutputObject =
      static_cast< MeasurementObjectType * >( this->ProcessObject::GetOutput(27) );
  secondMeasureOfInformationCorrelationOutputObject->Set(secondMeasureOfInformationCorrelation);
}

template< typename THistogram >
void
EnhancedHistogramToTextureFeaturesFilter< THistogram >::ComputeMeansAndVariances(double & pixelMean,
                                                                                 double & marginalMean,
                                                                                 double & marginalDevSquared,
                                                                                 double & pixelVariance)
{
  // This function takes two passes through the histogram and two passes through
  // an array of the same length as a histogram axis. This could probably be
  // cleverly compressed to one pass, but it's not clear that that's necessary.

  typedef typename HistogramType::ConstIterator HistogramIterator;

  const HistogramType *inputHistogram =  this->GetInput();

  // Initialize everything
  typename HistogramType::SizeValueType binsPerAxis = inputHistogram->GetSize(0);
  double *marginalSums = new double[binsPerAxis];
  for ( double *ms_It = marginalSums;
        ms_It < marginalSums + binsPerAxis; ms_It++ )
  {
    *ms_It = 0;
  }
  pixelMean = 0;

  typename RelativeFrequencyContainerType::const_iterator rFreqIterator =
      m_RelativeFrequencyContainer.begin();

  // Ok, now do the first pass through the histogram to get the marginal sums
  // and compute the pixel mean
  HistogramIterator hit = inputHistogram->Begin();
  while ( hit != inputHistogram->End() )
  {
    RelativeFrequencyType frequency = *rFreqIterator;
    IndexType             index = inputHistogram->GetIndex( hit.GetInstanceIdentifier() );
    pixelMean += index[0] * frequency;
    marginalSums[index[0]] += frequency;
    ++hit;
    ++rFreqIterator;
  }

  /*  Now get the mean and deviaton of the marginal sums.
      Compute incremental mean and SD, a la Knuth, "The  Art of Computer
      Programming, Volume 2: Seminumerical Algorithms",  section 4.2.2.
      Compute mean and standard deviation using the recurrence relation:
      M(1) = x(1), M(k) = M(k-1) + (x(k) - M(k-1) ) / k
      S(1) = 0, S(k) = S(k-1) + (x(k) - M(k-1)) * (x(k) - M(k))
      for 2 <= k <= n, then
      sigma = std::sqrt(S(n) / n) (or divide by n-1 for sample SD instead of
      population SD).
      */
  marginalMean = marginalSums[0];
  marginalDevSquared = 0;
  for ( unsigned int arrayIndex = 1; arrayIndex < binsPerAxis; arrayIndex++ )
  {
    int    k = arrayIndex + 1;
    double M_k_minus_1 = marginalMean;
    double S_k_minus_1 = marginalDevSquared;
    double x_k = marginalSums[arrayIndex];

    double M_k = M_k_minus_1 + ( x_k - M_k_minus_1 ) / k;
    double S_k = S_k_minus_1 + ( x_k - M_k_minus_1 ) * ( x_k - M_k );

    marginalMean = M_k;
    marginalDevSquared = S_k;
  }
  marginalDevSquared = marginalDevSquared / binsPerAxis;

  rFreqIterator = m_RelativeFrequencyContainer.begin();
  // OK, now compute the pixel variances.
  pixelVariance = 0;
  for ( hit = inputHistogram->Begin(); hit != inputHistogram->End(); ++hit )
  {
    RelativeFrequencyType frequency = *rFreqIterator;
    IndexType             index = inputHistogram->GetIndex( hit.GetInstanceIdentifier() );
    pixelVariance += ( index[0] - pixelMean ) * ( index[0] - pixelMean ) * frequency;
    ++rFreqIterator;
  }

  delete[] marginalSums;
}

template< typename THistogram >
const
typename EnhancedHistogramToTextureFeaturesFilter< THistogram >::MeasurementObjectType *
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::GetEnergyOutput() const
{
  return static_cast< const MeasurementObjectType * >( this->ProcessObject::GetOutput(0) );
}

template< typename THistogram >
const
typename EnhancedHistogramToTextureFeaturesFilter< THistogram >::MeasurementObjectType *
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::GetEntropyOutput() const
{
  return static_cast< const MeasurementObjectType * >( this->ProcessObject::GetOutput(1) );
}

template< typename THistogram >
const
typename EnhancedHistogramToTextureFeaturesFilter< THistogram >::MeasurementObjectType *
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::GetCorrelationOutput() const
{
  return static_cast< const MeasurementObjectType * >( this->ProcessObject::GetOutput(2) );
}

template< typename THistogram >
const
typename EnhancedHistogramToTextureFeaturesFilter< THistogram >::MeasurementObjectType *
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::GetInverseDifferenceMomentOutput() const
{
  return static_cast< const MeasurementObjectType * >( this->ProcessObject::GetOutput(3) );
}

template< typename THistogram >
const
typename EnhancedHistogramToTextureFeaturesFilter< THistogram >::MeasurementObjectType *
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::GetInertiaOutput() const
{
  return static_cast< const MeasurementObjectType * >( this->ProcessObject::GetOutput(4) );
}

template< typename THistogram >
const
typename EnhancedHistogramToTextureFeaturesFilter< THistogram >::MeasurementObjectType *
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::GetClusterShadeOutput() const
{
  return static_cast< const MeasurementObjectType * >( this->ProcessObject::GetOutput(5) );
}

template< typename THistogram >
const
typename EnhancedHistogramToTextureFeaturesFilter< THistogram >::MeasurementObjectType *
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::GetClusterProminenceOutput() const
{
  return static_cast< const MeasurementObjectType * >( this->ProcessObject::GetOutput(6) );
}

template< typename THistogram >
const
typename EnhancedHistogramToTextureFeaturesFilter< THistogram >::MeasurementObjectType *
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::GetHaralickCorrelationOutput() const
{
  return static_cast< const MeasurementObjectType * >( this->ProcessObject::GetOutput(7) );
}

itkMacroGLCMFeature(Autocorrelation,8)
itkMacroGLCMFeature(Contrast,9)
itkMacroGLCMFeature(Dissimilarity,10)
itkMacroGLCMFeature(MaximumProbability,11)
itkMacroGLCMFeature(InverseVariance,12)
itkMacroGLCMFeature(Homogeneity1,13)
itkMacroGLCMFeature(ClusterTendency,14)
itkMacroGLCMFeature(Variance,15)
itkMacroGLCMFeature(SumAverage,16)
itkMacroGLCMFeature(SumEntropy,17)
itkMacroGLCMFeature(SumVariance,18)
itkMacroGLCMFeature(DifferenceAverage,19)
itkMacroGLCMFeature(DifferenceEntropy,20)
itkMacroGLCMFeature(DifferenceVariance,21)
itkMacroGLCMFeature(InverseDifferenceMomentNormalized,22)
itkMacroGLCMFeature(InverseDifferenceNormalized,23)
itkMacroGLCMFeature(InverseDifference,24)
itkMacroGLCMFeature(JointAverage,25)
itkMacroGLCMFeature(FirstMeasureOfInformationCorrelation,26)
itkMacroGLCMFeature(SecondMeasureOfInformationCorrelation,27)

template< typename THistogram >
typename EnhancedHistogramToTextureFeaturesFilter< THistogram >::MeasurementType
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::GetEnergy() const
{
  return this->GetEnergyOutput()->Get();
}

template< typename THistogram >
typename EnhancedHistogramToTextureFeaturesFilter< THistogram >::MeasurementType
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::GetEntropy() const
{
  return this->GetEntropyOutput()->Get();
}

template< typename THistogram >
typename EnhancedHistogramToTextureFeaturesFilter< THistogram >::MeasurementType
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::GetCorrelation() const
{
  return this->GetCorrelationOutput()->Get();
}

template< typename THistogram >
typename EnhancedHistogramToTextureFeaturesFilter< THistogram >::MeasurementType
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::GetInverseDifferenceMoment() const
{
  return this->GetInverseDifferenceMomentOutput()->Get();
}

template< typename THistogram >
typename EnhancedHistogramToTextureFeaturesFilter< THistogram >::MeasurementType
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::GetInertia() const
{
  return this->GetInertiaOutput()->Get();
}

template< typename THistogram >
typename EnhancedHistogramToTextureFeaturesFilter< THistogram >::MeasurementType
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::GetClusterShade() const
{
  return this->GetClusterShadeOutput()->Get();
}

template< typename THistogram >
typename EnhancedHistogramToTextureFeaturesFilter< THistogram >::MeasurementType
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::GetClusterProminence() const
{
  return this->GetClusterProminenceOutput()->Get();
}

template< typename THistogram >
typename EnhancedHistogramToTextureFeaturesFilter< THistogram >::MeasurementType
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::GetHaralickCorrelation() const
{
  return this->GetHaralickCorrelationOutput()->Get();
}

#define itkMacroGLCMFeatureSwitch(name)     \
  case name :                             \
  return this->Get##name()
template< typename THistogram >
typename EnhancedHistogramToTextureFeaturesFilter< THistogram >::MeasurementType
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::GetFeature(TextureFeatureName feature)
{
  switch ( feature )
  {
  itkMacroGLCMFeatureSwitch(Energy);
  itkMacroGLCMFeatureSwitch(Entropy);
  itkMacroGLCMFeatureSwitch(Correlation);
  itkMacroGLCMFeatureSwitch(InverseDifferenceMoment);
  itkMacroGLCMFeatureSwitch(Inertia);
  itkMacroGLCMFeatureSwitch(ClusterShade);
  itkMacroGLCMFeatureSwitch(ClusterProminence);
  itkMacroGLCMFeatureSwitch(HaralickCorrelation);
  itkMacroGLCMFeatureSwitch(Autocorrelation);
  itkMacroGLCMFeatureSwitch(Contrast);
  itkMacroGLCMFeatureSwitch(Dissimilarity);
  itkMacroGLCMFeatureSwitch(MaximumProbability);
  itkMacroGLCMFeatureSwitch(InverseVariance);
  itkMacroGLCMFeatureSwitch(Homogeneity1);
  itkMacroGLCMFeatureSwitch(ClusterTendency);
  itkMacroGLCMFeatureSwitch(Variance);
  itkMacroGLCMFeatureSwitch(SumAverage);
  itkMacroGLCMFeatureSwitch(SumEntropy);
  itkMacroGLCMFeatureSwitch(SumVariance);
  itkMacroGLCMFeatureSwitch(DifferenceAverage);
  itkMacroGLCMFeatureSwitch(DifferenceEntropy);
  itkMacroGLCMFeatureSwitch(DifferenceVariance);
  itkMacroGLCMFeatureSwitch(InverseDifferenceMomentNormalized);
  itkMacroGLCMFeatureSwitch(InverseDifferenceNormalized);
  itkMacroGLCMFeatureSwitch(InverseDifference);
  itkMacroGLCMFeatureSwitch(JointAverage);
  itkMacroGLCMFeatureSwitch(FirstMeasureOfInformationCorrelation);
  itkMacroGLCMFeatureSwitch(SecondMeasureOfInformationCorrelation);
  default:
    return 0;
  }
}

#undef itkMacroGLCMFeatureSwitch

template< typename THistogram >
void
EnhancedHistogramToTextureFeaturesFilter< THistogram >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}
} // end of namespace Statistics
} // end of namespace itk

#endif
