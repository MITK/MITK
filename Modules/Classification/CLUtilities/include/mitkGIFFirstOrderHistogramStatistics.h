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
  * - <b>First Order Histogram::Mean Value</b>: The mean intensity of all voxels, calulated by \f$ \mu_x  = \sum p_i  x_i\f$.
  * - <b>First Order Histogram::Variance Value</b> The variance intensity is calculated as : \f$ \sigma^2  = \sum p_i (x_i - \mu_x)^2\f$.
  * - <b>First Order Histogram::Skewness Value</b>: \f[ skewness = \frac{\sum p_i (x_i - \mu_x)^3}{\sigma^3} \f]
  * - <b>First Order Histogram::Excess Kurtosis Value</b>: \f[ skewness = \frac{\sum p_i (x_i - \mu_x)^4}{\sigma^4} - 3 \f]
  * - <b>First Order Histogram::Median Value</b>: The median intensity value based on the histogram values.
  * - <b>First Order Histogram::Minimum Value</b>: The minimum observed intensity value.
  * - <b>First Order Histogram::Percentile 10 Value</b>: The intensity that is equal or greater than 10% of all observed intensities.
  * - <b>First Order Histogram::Percentile 90 Value</b>: The intensity that is equal or greater than 90% of all observed intensities.
  * - <b>First Order Histogram::Maximum Value</b>: The maximum observerd intensity value.
  * - <b>First Order Histogram::Mode Value</b>: The most common intensity value, i.e. the value of the bin with the highest probability.
  * - <b>First Order Histogram::Interquantile Range Value</b>: The intensity difference between Percentile 75% (\f$ P75\f$) and Percentile 25% (\f$ P25\f$).
  * - <b>First Order Histogram::Range Value</b>: The difference between the observed maximum and minimum intensity.
  * - <b>First Order Histogram::Mean Absolute Deviation Value</b>: \f[ \textup{mean absolute deviation} = \sum p_i \left \| (x_i - \mu_x) \right \| \f]
  * - <b>First Order Histogram::Robust Mean Value</b>: The mean of all intensities between the 10% and 90% quantile.
  * - <b>First Order Histogram::Robust Mean Absolute Deviation Value</b>: The Mean absolute deviation for all values between the 10% and 90% quantile. It is based on the robust mean value.
  * - <b>First Order Histogram::Median Absolute Deviation Value</b>: \f[ \textup{mean absolute deviation} = \sum p_i \left \| (x_i - \textup{median}) \right \| \f]
  * - <b>First Order Histogram::Coefficient of Variation Value</b>: \f[ \frac{\sigma_x}{\mu_x} \f]
  * - <b>First Order Histogram::Quantile coefficient of Dispersion Value</b>: \f[ \textup{Quantile coefficient of Dispersion} = \frac{P75 - P25}{P75 + P25} \f]
  * - <b>First Order Histogram::Entropy Value</b>: The entropy is only based on histogram bins with a probability greater than 0.0000001: \f[ \textup{entropy} = - \sum p_i \textup{log}_2 p_i \f]
  * - <b>First Order Histogram::Uniformity Value</b>: \f$ \sum p_i^2 \f$
  * - <b>First Order Histogram::Mean Index</b>: The mean index of all voxels, calulated by \f$ \mu_i  = \sum p_i  i\f$.
  * - <b>First Order Histogram::Variance Index</b>: The variance index is calculated as : \f$ \sigma_i^2  = \sum p_i (i - \mu_i)^2\f$.
  * - <b>First Order Histogram::Skewness Index</b>: \f[ skewness = \frac{\sum p_i (i - \mu_i)^3}{\sigma_i^3} \f]
  * - <b>First Order Histogram::Excess Kurtosis Index</b>: \f[ skewness = \frac{\sum p_i (i - \mu_i)^4}{\sigma_i^4} - 3 \f]
  * - <b>First Order Histogram::Median Index</b>: The median index value based on the histogram values.
  * - <b>First Order Histogram::Minimum Index</b>: The index of the minimum observed intensity value.
  * - <b>First Order Histogram::Percentile 10 Index</b>: The index oft the intensity that is equal or greater than 10% of all observed intensities.
  * - <b>First Order Histogram::Percentile 90 Index</b>: The index of the intensity that is equal or greater than 90% of all observed intensities.
  * - <b>First Order Histogram::Maximum Index</b>: The index of the maximum observerd intensity value.
  * - <b>First Order Histogram::Mode Index</b>: The index of the most common intensity value, i.e. the index of the bin with the highest probability.
  * - <b>First Order Histogram::Interquantile Range Index</b>: The index difference between Percentile 75% (\f$ P75\f$) and Percentile 25% (\f$ P25\f$).
  * - <b>First Order Histogram::Range Index</b>: The index difference between the index of the observed maximum and minimum intensity.
  * - <b>First Order Histogram::Mean Absolute Deviation Index</b>: \f[ \textup{mean absolute deviation} = \sum p_i \left \| (i - \mu_i) \right \| \f]
  * - <b>First Order Histogram::Robust Mean Absolute Deviation Index</b>: The Mean absolute deviation for all values between the 10% and 90% quantile. It is based on the robust mean value.
  * - <b>First Order Histogram::Median Absolute Deviation Index</b>: \f[ \textup{mean absolute deviation} = \sum p_i \left \| (i - \textup{median}) \right \| \f]
  * - <b>First Order Histogram::Coefficient of Variation Index</b>: \f[ \frac{\sigma_i}{\mu_i} \f]
  * - <b>First Order Histogram::Quantile coefficient of Dispersion Index</b>: \f[ \textup{Quantile coefficient of Dispersion} = \frac{P75 - P25}{P75 + P25} \f]
  * - <b>First Order Histogram::Entropy Index</b>: The entropy is only based on histogram bins with a probability greater than 0.0000001: \f$ \textup{entropy} = - \sum p_i \textup{log}_2 p_i \f$. Note that this is the same as the entropy value.
  * - <b>First Order Histogram::Uniformity Index</b>: \f$ \sum p_i^2 \f$. Note that this is the same as the uniformity value.
  * - <b>First Order Histogram::Maximum Gradient</b>: The maximum difference between the probability of three neighbouring bins. For bins at the edge of the histogram, only two bins are used for the calulation.
  * - <b>First Order Histogram::Maximum Gradient Index</b>: The index of the bin that belongs to the maximum gradient.
  * - <b>First Order Histogram::Minimum Gradient</b>: The minimum difference between the probability of three neighbouring bins. For bins at the edge of the histogram, only two bins are used for the calulation.
  * - <b>First Order Histogram::Minimum Gradient Index</b>:The index of the bin that belongs to the minimum gradient.
  * - <b>First Order Histogram::Robust Mean Index</b>: The mean index of all intensities between the 10% and 90% quantile.
  * - <b>First Order Histogram::Number of Bins</b>: The number of bins in the histogram. This is rather for control, as this parameter is likely to be determined by the configuration rather than the image.
  * - <b>First Order Histogram::Bin Size</b>: The binsize of the bins from the histogram. This is rather for control, as this parameter is likely to be determined by the configuration rather than the image.
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

    virtual void CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList);
    virtual void AddArguments(mitkCommandLineParser &parser);
    virtual std::string GetCurrentFeatureEncoding() override;


    struct ParameterStruct {
      double MinimumIntensity;
      double MaximumIntensity;
      int Bins;
      std::string prefix;
    };

  private:
    double m_Range;
  };
}
#endif //mitkGIFFirstOrderHistogramStatistics_h
