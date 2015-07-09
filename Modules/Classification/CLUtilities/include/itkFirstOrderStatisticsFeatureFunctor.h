#ifndef itkNeighborhoodFirstOrderStatistics_h
#define itkNeighborhoodFirstOrderStatistics_h

#include "itkConstNeighborhoodIterator.h"

namespace itk
{

namespace Functor
{

template< typename TNeighborhoodType, typename TPixelOutputType >
struct NeighborhoodFirstOrderStatistics
{
  static const unsigned int OutputCount = 6;
  typedef vnl_vector_fixed<TPixelOutputType, OutputCount>  OutputVectorType;
  typedef TNeighborhoodType                                NeighborhoodType;
  enum OutputFeatures
  {
    MEAN, VARIANCE, SKEWNESS, KURTOSIS, MIN, MAX
  };

  NeighborhoodFirstOrderStatistics(){std::cout << "NeighborhoodFirstOrderStatistics" << std::endl;}

  static const char * GetFeatureName(unsigned int f )
  {
    static const char * const FeatureNames[] = {"MEAN","VARIANCE","SKEWNESS", "KURTOSIS", "MIN", "MAX"};
    return FeatureNames[f];
  }

  inline OutputVectorType operator()(const NeighborhoodType & it) const
  {
    double mean = 0;
    double sqmean = 0;
    double min = 10000000;
    double max = -10000000;

    for (int i = 0; i < it.Size(); ++i)
    {
      double value = it.GetPixel(i);
      mean += value;
      sqmean += (value * value);
      max = (value > max) ? value : max;
      min = (value < min) ? value : min;
    }
    mean /= it.Size();
    sqmean /= it.Size();

    double variance = sqmean - mean * mean;

    double stdderivation = std::sqrt(variance);
    double skewness = 0;
    double kurtosis = 0;

    for (int i = 0; i < it.Size(); ++i)
    {
      double value = it.GetPixel(i);
      double tmpskewness = (value - mean) / stdderivation;
      skewness += tmpskewness * tmpskewness * tmpskewness;
      kurtosis += (value - mean) * (value - mean) * (value - mean) * (value - mean);
    }
    skewness /= it.Size();
    kurtosis /= it.Size();
    kurtosis /= (variance * variance);

    if(skewness!=skewness){
        skewness = 0;
    }
    if(kurtosis!=kurtosis){
        kurtosis = 0;
    }

    OutputVectorType output_vector;
    output_vector[MEAN] = mean;
    output_vector[VARIANCE] = variance;
    output_vector[SKEWNESS] = skewness;
    output_vector[KURTOSIS] = kurtosis;
    output_vector[MIN] = min;
    output_vector[MAX] = max;

    return output_vector;

  }
};

}// end namespace functor

} // end namespace itk
#endif
