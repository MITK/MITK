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

#ifndef mitkGIFFirstOrderHistogramStatistics_h
#define mitkGIFFirstOrderHistogramStatistics_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

namespace mitk
{
  /**
  * \brief Calulates first order features based on a histogram.
  *
  * This class can be used to calculate first order features based on a histogram.
  * For each feature, two variations are given, once the value of the feature that is
  * obtained if the mean intensity of the histogram bins is used and the
  * histogram bin that corresponds to the feature value. See AbstractGlobalImageFeature for more
  * information on the histogram initialization. The histogram gives a probability \f$p_i\f$ for the
  * intensity \f$x_i\f$ that is linked to the bin \f$i\f$. The histogram bins start at index 1.
  *
  * This feature calculator is activated by the option "<b>-first-order-histogram</b>" or "<b>-foh</b>".
  * Beside the options for the histogram definitions, which are given in the description of AbstractGlobalImageFeature , no
  * additional parameters are available.
  *
  * The features are calculated based on a mask. It is assumed that the mask is
  * of the type of an unsigned short image and all voxels with an value of 1
  * are treated as masked.
  *
  * The resulting features are:
  * - <b>FirstOrderHistogram::Mean Value</b>: The mean intensity of all voxels, calulated by \f$ \mu_x  = \sum p_i  x_i\f$.
  * - <b>FirstOrderHistogram::Variance Value</b> The variance intensity is calculated as : \f$ \sigma^2  = \sum p_i (x_i - \mu_x)^2\f$.
  * - <b>FirstOrderHistogram::Skewness Value</b>: \f[ skewness = \frac{\sum p_i (x_i - \mu_x)^3}{\sigma^3} \f]
  * - <b>FirstOrderHistogram::Excess Kurtosis Value</b>: \f[ skewness = \frac{\sum p_i (x_i - \mu_x)^4}{\sigma^4} - 3 \f]
  * - <b>FirstOrderHistogram::Median Value</b>: The median intensity value based on the histogram values.
  * - <b>FirstOrderHistogram::Minimum Value</b>: The minimum observed intensity value.
  * - <b>FirstOrderHistogram::Percentile 10 Value</b>: The intensity that is equal or greater than 10% of all observed intensities.
  * - <b>FirstOrderHistogram::Percentile 90 Value</b>: The intensity that is equal or greater than 90% of all observed intensities.
  * - <b>FirstOrderHistogram::Maximum Value</b>: The maximum observerd intensity value.
  * - <b>FirstOrderHistogram::Mode Value</b>: The most common intensity value, i.e. the value of the bin with the highest probability.
  * - <b>FirstOrderHistogram::Interquantile Range Value</b>: The intensity difference between Percentile 75% (\f$ P75\f$) and Percentile 25% (\f$ P25\f$).
  * - <b>FirstOrderHistogram::Range Value</b>: The difference between the observed maximum and minimum intensity.
  * - <b>FirstOrderHistogram::Mean Absolute Deviation Value</b>: \f[ \textup{mean absolute deviation} = \sum p_i \left \| (x_i - \mu_x) \right \| \f]
  * - <b>FirstOrderHistogram::Robust Mean Value</b>: The mean of all intensities between the 10% and 90% quantile.
  * - <b>FirstOrderHistogram::Robust Mean Absolute Deviation Value</b>: The Mean absolute deviation for all values between the 10% and 90% quantile. It is based on the robust mean value.
  * - <b>FirstOrderHistogram::Median Absolute Deviation Value</b>: \f[ \textup{mean absolute deviation} = \sum p_i \left \| (x_i - \textup{median}) \right \| \f]
  * - <b>FirstOrderHistogram::Coefficient of Variation Value</b>: \f[ \frac{\sigma_x}{\mu_x} \f]
  * - <b>FirstOrderHistogram::Quantile coefficient of Dispersion Value</b>: \f[ \textup{Quantile coefficient of Dispersion} = \frac{P75 - P25}{P75 + P25} \f]
  * - <b>FirstOrderHistogram::Entropy Value</b>: The entropy is only based on histogram bins with a probability greater than 0.0000001: \f[ \textup{entropy} = - \sum p_i \textup{log}_2 p_i \f]
  * - <b>FirstOrderHistogram::Uniformity Value</b>: \f$ \sum p_i^2 \f$
  * - <b>FirstOrderHistogram::Mean Index</b>: The mean index of all voxels, calulated by \f$ \mu_i  = \sum p_i  i\f$.
  * - <b>FirstOrderHistogram::Variance Index</b>: The variance index is calculated as : \f$ \sigma_i^2  = \sum p_i (i - \mu_i)^2\f$.
  * - <b>FirstOrderHistogram::Skewness Index</b>: \f[ skewness = \frac{\sum p_i (i - \mu_i)^3}{\sigma_i^3} \f]
  * - <b>FirstOrderHistogram::Excess Kurtosis Index</b>: \f[ skewness = \frac{\sum p_i (i - \mu_i)^4}{\sigma_i^4} - 3 \f]
  * - <b>FirstOrderHistogram::Median Index</b>: The median index value based on the histogram values.
  * - <b>FirstOrderHistogram::Minimum Index</b>: The index of the minimum observed intensity value.
  * - <b>FirstOrderHistogram::Percentile 10 Index</b>: The index oft the intensity that is equal or greater than 10% of all observed intensities.
  * - <b>FirstOrderHistogram::Percentile 90 Index</b>: The index of the intensity that is equal or greater than 90% of all observed intensities.
  * - <b>FirstOrderHistogram::Maximum Index</b>: The index of the maximum observerd intensity value.
  * - <b>FirstOrderHistogram::Mode Index</b>: The index of the most common intensity value, i.e. the index of the bin with the highest probability.
  * - <b>FirstOrderHistogram::Interquantile Range Index</b>: The index difference between Percentile 75% (\f$ P75\f$) and Percentile 25% (\f$ P25\f$).
  * - <b>FirstOrderHistogram::Range Index</b>: The index difference between the index of the observed maximum and minimum intensity.
  * - <b>FirstOrderHistogram::Mean Absolute Deviation Index</b>: \f[ \textup{mean absolute deviation} = \sum p_i \left \| (i - \mu_i) \right \| \f]
  * - <b>FirstOrderHistogram::Robust Mean Absolute Deviation Index</b>: The Mean absolute deviation for all values between the 10% and 90% quantile. It is based on the robust mean value.
  * - <b>FirstOrderHistogram::Median Absolute Deviation Index</b>: \f[ \textup{mean absolute deviation} = \sum p_i \left \| (i - \textup{median}) \right \| \f]
  * - <b>FirstOrderHistogram::Coefficient of Variation Index</b>: \f[ \frac{\sigma_i}{\mu_i} \f]
  * - <b>FirstOrderHistogram::Quantile coefficient of Dispersion Index</b>: \f[ \textup{Quantile coefficient of Dispersion} = \frac{P75 - P25}{P75 + P25} \f]
  * - <b>FirstOrderHistogram::Entropy Index</b>: The entropy is only based on histogram bins with a probability greater than 0.0000001: \f$ \textup{entropy} = - \sum p_i \textup{log}_2 p_i \f$. Note that this is the same as the entropy value.
  * - <b>FirstOrderHistogram::Uniformity Index</b>: \f$ \sum p_i^2 \f$. Note that this is the same as the uniformity value.
  * - <b>FirstOrderHistogram::Maximum Gradient</b>: The maximum difference between the probability of three neighbouring bins. For bins at the edge of the histogram, only two bins are used for the calulation.
  * - <b>FirstOrderHistogram::Maximum Gradient Index</b>: The index of the bin that belongs to the maximum gradient.
  * - <b>FirstOrderHistogram::Minimum Gradient</b>: The minimum difference between the probability of three neighbouring bins. For bins at the edge of the histogram, only two bins are used for the calulation.
  * - <b>FirstOrderHistogram::Minimum Gradient Index</b>:The index of the bin that belongs to the minimum gradient.
  * - <b>FirstOrderHistogram::Robust Mean Index</b>: The mean index of all intensities between the 10% and 90% quantile.
  * - <b>FirstOrderHistogram::Number of Bins</b>: The number of bins in the histogram. This is rather for control, as this parameter is likely to be determined by the configuration rather than the image.
  * - <b>FirstOrderHistogram::Bin Size</b>: The binsize of the bins from the histogram. This is rather for control, as this parameter is likely to be determined by the configuration rather than the image.
  */
  class MITKCLUTILITIES_EXPORT GIFFirstOrderHistogramStatistics : public AbstractGlobalImageFeature
  {
  public:
    mitkClassMacro(GIFFirstOrderHistogramStatistics,AbstractGlobalImageFeature)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFFirstOrderHistogramStatistics();

    /**
    * \brief Calculates the Cooccurence-Matrix based features for this class.
    */
    virtual FeatureListType CalculateFeatures(const Image::Pointer & image, const Image::Pointer &feature) override;

    /**
    * \brief Returns a list of the names of all features that are calculated from this class
    */
    virtual FeatureNameListType GetFeatureNames() override;

    itkGetConstMacro(Range,double);
    itkSetMacro(Range, double);
    itkGetConstMacro(HistogramSize,int);
    itkSetMacro(HistogramSize, int);
    itkGetConstMacro(UseCtRange,bool);
    itkSetMacro(UseCtRange, bool);
    itkGetConstMacro(BinSize, double);
    itkSetMacro(BinSize, double);

    virtual void CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList);
    virtual void AddArguments(mitkCommandLineParser &parser);


    struct ParameterStruct {
      double MinimumIntensity;
      double MaximumIntensity;
      int Bins;
    };

  private:
    double m_Range;
    int m_HistogramSize;
    bool m_UseCtRange;
    double m_BinSize;
  };
}
#endif //mitkGIFFirstOrderHistogramStatistics_h
